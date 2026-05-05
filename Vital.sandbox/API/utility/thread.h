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
            int id {};
            std::string env;
            std::atomic<bool> destroyed { false };
            std::atomic<bool> sleeping { false };
            std::atomic<bool> awaiting { false };
            std::atomic<bool> vm_owned { true };
            Machine* vm = nullptr;
            Machine* thread_vm = nullptr;
            void** userdata = nullptr;
            std::string reference() const { return fmt::format("{}:{}", base_name, id); }
            std::string self_reference() const { return fmt::format("{}:{}:self", base_name, id); }
        };
        inline static std::mutex mutex;
        inline static std::unordered_map<int, std::shared_ptr<Instance>> buffer;
        inline static std::atomic<int> next_id { 1 };

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

            if (instance -> vm_owned.exchange(false)) {
                Machine* tvm = instance -> thread_vm;
                instance -> thread_vm = nullptr;
                if (tvm) delete tvm;
            }
            else instance -> thread_vm = nullptr;
            if (instance -> vm) {
                vm_module::release_userdata_ptr(instance -> userdata);
                instance -> vm -> del_reference(instance -> self_reference());
                instance -> vm -> del_reference(instance -> reference());
                instance -> vm = nullptr;
            }
        }

        static bool safe_resume(std::shared_ptr<Instance> instance, int args) {
            if (!instance || instance -> destroyed) return false;
            if (!instance -> vm_owned.load()) return false;
            if (!instance -> thread_vm) return false;
            if (!instance -> vm) {
                Machine* tvm = instance -> thread_vm;
                instance -> thread_vm = nullptr;
                instance -> vm_owned.store(false);
                if (tvm) delete tvm;
                clean_instance(instance);
                return false;
            }

            vm_state* raw_state = instance -> thread_vm -> get_state();
            if (!raw_state) return false;
            {
                int status = lua_status(raw_state);
                if (status != LUA_OK && status != LUA_YIELD) {
                    instance -> thread_vm = nullptr;
                    instance -> vm_owned.store(false);
                    clean_instance(instance);
                    return false;
                }
            }
            instance -> vm_owned.store(false);
            instance -> thread_vm -> resume(args);
            if (lua_status(raw_state) != LUA_YIELD) {
                instance -> thread_vm = nullptr;
                clean_instance(instance);
                return false;
            }
            instance -> vm_owned.store(true);
            return true;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Thread>(vm, base_name);

            Promise::register_resume_dispatcher([](int thread_id, bool resolved, std::shared_ptr<Promise::Instance> promise) {
                auto instance = Thread::fetch_instance(thread_id);
                if (!instance || instance -> destroyed) return;
                if (!instance -> vm_owned.load()) return;
                if (!instance -> thread_vm) return;

                instance -> thread_vm -> push_bool(resolved);
                int value_count = Promise::push_values(promise, instance -> thread_vm);
                instance -> awaiting = false;
                safe_resume(instance, 1 + value_count);
            });

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(exec)")
                    .require(1, &Machine::is_function);

                auto instance = std::make_shared<Instance>();
                instance -> id  = next_id.fetch_add(1);
                instance -> env = vm -> get_environment_id();
                instance -> vm  = vm;
                Machine* thread_vm = vm -> create_thread();
                instance -> thread_vm = thread_vm;
                vm -> set_reference(instance -> reference(), 1);
                vm -> pop(1);
                vm -> create_object(base_name, instance.get());
                instance -> userdata = vm_module::get_userdata_ptr(vm, -1);
                vm -> set_reference(instance -> self_reference(), -1);
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    buffer[instance -> id] = instance;
                }
                vm -> get_reference(instance -> self_reference(), true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, base_name, "resume", [](auto vm, auto self, auto& id) -> int {
                if (self -> destroyed || !self -> thread_vm || self -> sleeping || self -> awaiting) {
                    vm -> push_value(false); return 1;
                }
                auto instance = fetch_instance(self -> id);
                if (!instance) { vm -> push_value(false); return 1; }

                self -> vm -> get_reference(self -> reference(), true);
                self -> vm -> move(self -> thread_vm, 1);
                self -> vm -> get_reference(self -> self_reference(), true);
                self -> vm -> move(self -> thread_vm, 1);
                safe_resume(instance, 1);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "destroy", [](auto vm, auto self, auto& id) -> int {
                if (self -> destroyed) { vm -> push_value(false); return 1; }
                auto instance = fetch_instance(self -> id);
                if (instance) clean_instance(instance);
                vm_module::release_userdata(vm, 1);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "is_instance", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(!self -> destroyed);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "get_thread", [](auto vm, auto self, auto& id) -> int {
                if (self -> destroyed || !self -> thread_vm) { vm -> push_value(false); return 1; }
                vm -> push_value(self -> thread_vm -> get_state() == vm -> get_state());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "pause", [](auto vm, auto self, auto& id) -> int {
                if (self -> destroyed || !vm -> is_virtual()) { vm -> push_value(false); return 1; }
                return lua_yieldk(vm -> get_state(), 0, 0, [](lua_State*, int, lua_KContext) -> int { return 0; });
            });

            vm_module::bind_method<Instance>(vm, base_name, "sleep", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(duration)")
                    .require(2, &Machine::is_number)
                    .validate(2, [](auto vm, int index) { return vm -> get_int(index) >= 0; }, "expected >= 0");

                if (self -> destroyed || self -> sleeping || self -> awaiting) { vm -> push_value(false); return 1; }

                int duration = vm -> get_int(2);
                self -> sleeping = true;
                auto instance = fetch_instance(self -> id);
                auto weak = std::weak_ptr<Instance>(instance);
                Tool::Timer::create([weak](Tool::Timer*, int) {
                    auto inst = weak.lock();
                    if (!inst || inst -> destroyed) return;
                    inst -> sleeping = false;
                    if (!inst -> vm_owned.load() || !inst -> thread_vm) return;
                    safe_resume(inst, 0);
                }, duration, 1);
                return lua_yieldk(vm -> get_state(), 0, 0, [](lua_State*, int, lua_KContext) -> int { return 0; });
            });

            vm_module::bind_method<Instance>(vm, base_name, "await", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(promise)")
                    .require(2, [](Machine* vm, int index) {
                        return vm_module::is_userdata(vm, Promise::base_name, index);
                    });

                if (self -> destroyed || self -> sleeping || self -> awaiting) { vm -> push_value(false); return 1; }
                if (!vm -> is_virtual()) { vm -> push_value(false); return 1; }

                auto ud = static_cast<Promise::Instance**>(vm -> get_userdata(2));
                if (!ud || !*ud) { vm -> push_value(false); return 1; }
                auto promise_inst = Promise::fetch_instance((*ud) -> id);
                if (!promise_inst) { vm -> push_value(false); return 1; }

                if (promise_inst -> state != Promise::State::Pending) {
                    vm -> push_bool(promise_inst -> resolved);
                    int n = Promise::push_values(promise_inst, vm);
                    return 1 + n;
                }
                self -> awaiting = true;
                promise_inst -> waiting.push_back({ self -> id });

                struct AwaitCTX { int base; int thread_id; };
                auto actx = new AwaitCTX { vm -> get_count(), self -> id };
                lua_KContext ctx = reinterpret_cast<lua_KContext>(actx);
                return lua_yieldk(vm -> get_state(), 0, ctx, [](lua_State* L, int, lua_KContext ctx) -> int {
                    auto actx = reinterpret_cast<AwaitCTX*>(ctx);
                    auto inst = Thread::fetch_instance(actx -> thread_id);
                    if (inst) inst -> awaiting = false;
                    int n = lua_gettop(L) - actx -> base;
                    delete actx;
                    return n > 0 ? n : 0;
                });
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
            for (auto& instance : to_clean) {
                instance -> destroyed = true;
                clean_instance(instance);
            }
        }
    };
}