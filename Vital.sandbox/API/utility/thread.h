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
            Machine* vm = nullptr;
            Machine* thread_vm = nullptr;
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
            instance -> vm -> del_reference(instance -> reference());
        }

        static void bind(Machine* vm) {
            // thread.create(exec) -> thread_id
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

                vm -> push_number(instance -> id);
                return 1;
            });

            // thread.resume(thread_id)
            API::bind(vm, {base_name}, "resume", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(thread_id)")
                    .require(1, &Machine::is_number);

                int inst_id = vm -> get_int(1);
                auto instance = fetch_instance(inst_id);
                if (!instance || instance -> destroyed || !instance -> thread_vm) {
                    vm -> push_value(false);
                    return 1;
                }

                instance -> vm -> get_reference(instance -> reference(), true);
                instance -> thread_vm -> move(instance -> thread_vm, 1);
                instance -> thread_vm -> resume(1);

                if (lua_status(instance -> thread_vm -> get_state()) != LUA_YIELD) {
                    clean_instance(instance);
                }
                vm -> push_value(true);
                return 1;
            });

            // thread.destroy(thread_id)
            API::bind(vm, {base_name}, "destroy", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(thread_id)")
                    .require(1, &Machine::is_number);

                int inst_id = vm -> get_int(1);
                auto instance = fetch_instance(inst_id);
                if (!instance) { vm -> push_value(false); return 1; }
                clean_instance(instance);
                vm -> push_value(true);
                return 1;
            });

            // thread.get_thread() -> thread_id or false
            API::bind(vm, {base_name}, "get_thread", [](auto vm, auto& id) -> int {
                if (!vm -> is_virtual()) { vm -> push_value(false); return 1; }
                std::lock_guard<std::mutex> lock(mutex);
                for (auto& [inst_id, instance] : buffer) {
                    if (instance -> thread_vm && instance -> thread_vm -> get_state() == vm -> get_state()) {
                        vm -> push_number(inst_id);
                        return 1;
                    }
                }
                vm -> push_value(false);
                return 1;
            });

            // thread.pause()
            API::bind(vm, {base_name}, "pause", [](auto vm, auto& id) -> int {
                if (!vm -> is_virtual()) { vm -> push_value(false); return 1; }
                vm -> pause();
                return 0;
            });

            // thread.sleep(thread_id, duration)
            API::bind(vm, {base_name}, "sleep", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(thread_id, duration)")
                    .require(1, &Machine::is_number)
                    .require(2, &Machine::is_number)
                    .validate(2, [](auto vm, int index) { return vm -> get_int(index) >= 0; }, "expected >= 0");

                int inst_id  = vm -> get_int(1);
                int duration = vm -> get_int(2);

                auto instance = fetch_instance(inst_id);
                if (!instance || instance -> destroyed || instance -> sleeping) {
                    vm -> push_value(false);
                    return 1;
                }
                instance -> sleeping = true;
                auto weak = std::weak_ptr<Instance>(instance);

                Tool::Timer::create([weak](Tool::Timer* self, int) {
                    Vital::Engine::Core::get_singleton() -> push_deferred([weak]() {
                        auto instance = weak.lock();
                        if (!instance || instance -> destroyed) return;
                        instance -> sleeping = false;
                        if (!instance -> thread_vm) return;
                        instance -> thread_vm -> resume(0);
                        if (lua_status(instance -> thread_vm -> get_state()) != LUA_YIELD) {
                            clean_instance(instance);
                        }
                    });
                }, duration, 1);

                vm -> pause();
                return 0;
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