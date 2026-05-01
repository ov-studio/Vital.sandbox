/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: thread.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Thread APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/API/utility/promise.h>


/////////////////////////
// Vital: API: Thread //
/////////////////////////

namespace Vital::Sandbox::API {
    struct Thread : vm_module {
        inline static const std::string base_name = "thread";

        struct Instance {
            int id;
            std::string env;
            std::atomic<bool> destroyed{false};
            std::atomic<bool> sleeping{false};
            std::atomic<bool> awaiting{false};
            Machine* vm = nullptr;
            Machine* thread_vm = nullptr;
            void** userdata = nullptr;
            std::string reference() const { return fmt::format("{}:{}", base_name, id); }
        };
        inline static std::mutex mutex;
        inline static std::unordered_map<int, std::shared_ptr<Instance>> buffer;
        inline static std::atomic<int> next_id{1};

        static std::shared_ptr<Instance> fetch_instance(int id) {
            std::lock_guard<std::mutex> lock(mutex);
            auto it = buffer.find(id);
            return it != buffer.end() ? it -> second : nullptr;
        }

        static void clean_instance(std::shared_ptr<Instance> instance) {
            if (!instance) return;
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (buffer.find(instance -> id) == buffer.end()) return;
                buffer.erase(instance -> id);
            }
            instance -> destroyed = true;
            if (instance -> thread_vm) {
                delete instance -> thread_vm;
                instance -> thread_vm = nullptr;
            }
            vm_module::release_userdata_ptr(instance -> userdata);
            instance -> vm -> del_reference(instance -> reference());
        }

        // Safe resume helper — reports errors through owner vm, cleans up if dead
        static bool safe_resume(std::shared_ptr<Instance> instance, int args) {
            if (!instance || instance -> destroyed || !instance -> thread_vm) return false;
            instance -> thread_vm -> resume(args);
            int status = lua_status(instance -> thread_vm -> get_state());
            if (status != LUA_YIELD) {
                if (status != LUA_OK) {
                    std::string err = lua_tostring(instance -> thread_vm -> get_state(), -1);
                    lua_pop(instance -> thread_vm -> get_state(), 1);
                    API::log(std::string(Tool::Log::error::label), err);
                }
                clean_instance(instance);
                return false;
            }
            return true;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Thread>(vm, base_name);

            // thread.create(exec) -> thread userdata
            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(exec)")
                    .require(1, &Machine::is_function);

                std::string env = vm -> get_environment_id();
                auto instance = std::make_shared<Instance>();
                instance -> id = next_id.fetch_add(1);
                instance -> env = env;
                instance -> vm = vm;

                Machine* thread_vm = vm -> create_thread();
                instance -> thread_vm = thread_vm;

                vm -> push(1);
                vm -> set_reference(instance -> reference(), -1);
                vm -> pop(1);

                {
                    std::lock_guard<std::mutex> lock(mutex);
                    buffer[instance -> id] = instance;
                }

                vm -> create_object(base_name, instance.get());
                instance -> userdata = vm_module::get_userdata_ptr(vm, -1);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            // thread:resume()
            vm_module::bind_method<Instance>(vm, base_name, "resume", [](auto vm, auto self, auto& id) -> int {
                if (!self || self -> destroyed || !self -> thread_vm) { vm -> push_value(false); return 1; }

                auto instance = fetch_instance(self -> id);
                if (!instance) { vm -> push_value(false); return 1; }

                // Push exec function onto thread stack
                self -> vm -> get_reference(self -> reference(), true);
                self -> thread_vm -> move(self -> thread_vm, 1);

                // Push self (thread userdata) as first argument to exec
                void** ud = vm_module::get_userdata_ptr(vm, 1);
                lua_pushlightuserdata(self -> thread_vm -> get_state(), ud ? *ud : nullptr);
                luaL_setmetatable(self -> thread_vm -> get_state(), base_name.c_str());

                safe_resume(instance, 2); // exec(self)
                vm -> push_value(true);
                return 1;
            });

            // thread:destroy()
            vm_module::bind_method<Instance>(vm, base_name, "destroy", [](auto vm, auto self, auto& id) -> int {
                if (!self || self -> destroyed) { vm -> push_value(false); return 1; }
                auto instance = fetch_instance(self -> id);
                if (instance) Vital::Engine::Core::get_singleton() -> push_deferred([instance]() { clean_instance(instance); });
                vm_module::release_userdata(vm, 1);
                vm -> push_value(true);
                return 1;
            });

            // thread:is_instance()
            vm_module::bind_method<Instance>(vm, base_name, "is_instance", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self && !self -> destroyed);
                return 1;
            });

            // thread:get_thread() -> bool (are we running inside this thread?)
            vm_module::bind_method<Instance>(vm, base_name, "get_thread", [](auto vm, auto self, auto& id) -> int {
                if (!self || self -> destroyed || !self -> thread_vm) { vm -> push_value(false); return 1; }
                vm -> push_value(self -> thread_vm -> get_state() == vm -> get_state());
                return 1;
            });

            // thread:pause()
            vm_module::bind_method<Instance>(vm, base_name, "pause", [](auto vm, auto self, auto& id) -> int {
                if (!self || self -> destroyed || !vm -> is_virtual()) { vm -> push_value(false); return 1; }
                vm -> pause();
                return 0;
            });

            // thread:sleep(duration)
            vm_module::bind_method<Instance>(vm, base_name, "sleep", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(duration)")
                    .require(2, &Machine::is_number)
                    .validate(2, [](auto vm, int index) { return vm -> get_int(index) >= 0; }, "expected >= 0");

                if (!self || self -> destroyed || self -> sleeping) { vm -> push_value(false); return 1; }

                int duration = vm -> get_int(2);
                self -> sleeping = true;

                auto instance = fetch_instance(self -> id);
                auto weak = std::weak_ptr<Instance>(instance);

                Tool::Timer::create([weak](Tool::Timer* self, int) {
                    Vital::Engine::Core::get_singleton() -> push_deferred([weak]() {
                        auto instance = weak.lock();
                        if (!instance || instance -> destroyed) return;
                        instance -> sleeping = false;
                        if (!instance -> thread_vm) return;
                        safe_resume(instance, 0);
                    });
                }, duration, 1);

                vm -> pause();
                return 0;
            });

            // thread:await(promise) -> resolved, ...values
            vm_module::bind_method<Instance>(vm, base_name, "await", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(promise)")
                    .require(2, [](Machine* vm, int index) {
                        return vm_module::is_userdata(vm, Promise::base_name, index);
                    });

                if (!self || self -> destroyed || self -> awaiting) { vm -> push_value(false); return 1; }
                if (!vm -> is_virtual()) { vm -> push_value(false); return 1; }

                auto ud = static_cast<Promise::Instance**>(lua_touserdata(vm -> get_state(), 2));
                if (!ud || !*ud) { vm -> push_value(false); return 1; }
                auto promise_inst = Promise::fetch_instance((*ud) -> id);
                if (!promise_inst) { vm -> push_value(false); return 1; }

                // Already settled — return immediately without suspending
                if (promise_inst -> state != Promise::State::Pending) {
                    vm -> push_value(promise_inst -> resolved);
                    if (promise_inst -> result_ref != LUA_NOREF) {
                        lua_rawgeti(promise_inst -> vm -> get_state(), LUA_REGISTRYINDEX, promise_inst -> result_ref);
                        int n = (int)lua_rawlen(promise_inst -> vm -> get_state(), -1);
                        for (int i = 1; i <= n; ++i) lua_rawgeti(promise_inst -> vm -> get_state(), -(n - i + 2), i);
                        lua_remove(promise_inst -> vm -> get_state(), -(n + 1));
                        return 1 + n;
                    }
                    return 1;
                }

                // Register this thread as waiting on the promise
                self -> awaiting = true;
                auto instance = fetch_instance(self -> id);
                promise_inst -> waiting.push_back({ self -> thread_vm, self -> vm });

                // Suspend — promise settle() will resume with (resolved_bool, result_table)
                vm -> pause();

                // After resume: unpack result_table into return values
                bool resolved = vm -> get_bool(1);
                int n = 0;
                if (vm -> is_table(2)) {
                    n = vm -> get_length(2);
                    for (int i = 1; i <= n; ++i) vm -> get_table_field(i, 2);
                }

                if (instance) instance -> awaiting = false;
                return 1 + n;
            });
        }

        static void clean(const std::string& env) {
            std::vector<std::shared_ptr<Instance>> to_clean;
            {
                std::lock_guard<std::mutex> lock(mutex);
                for (auto& [id, instance] : buffer) {
                    if (instance -> env == env) to_clean.push_back(instance);
                }
            }
            for (auto& instance : to_clean) clean_instance(instance);
        }
    };
}