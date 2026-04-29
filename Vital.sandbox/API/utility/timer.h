/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: timer.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Timer APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/console.h>


////////////////////////
// Vital: API: Timer //
////////////////////////

namespace Vital::Sandbox::API {
    struct Timer : vm_module {
        inline static const std::string base_name = "timer";

        struct Instance {
            int id;
            std::string env_id;
            std::atomic<bool> destroyed{false};
            Machine* vm = nullptr;
            std::string ref_key() const { return fmt::format("{}:{}", base_name, id); }
        };
        inline static std::mutex mutex;
        inline static std::unordered_map<int, std::shared_ptr<Instance>> registry;
        inline static std::atomic<int> next_id{1};

        static void cancel_env(const std::string& env_id) {
            std::lock_guard<std::mutex> lock(mutex);
            for (auto& [id, inst] : registry) {
                if (inst -> env_id == env_id) inst -> destroyed = true;
            }
        }

        static std::shared_ptr<Instance> get_inst(int id) {
            std::lock_guard<std::mutex> lock(mutex);
            auto it = registry.find(id);
            return it != registry.end() ? it -> second : nullptr;
        }

        static void cleanup(std::shared_ptr<Instance> inst) {
            if (!inst) return;
            inst -> vm -> del_reference(inst -> ref_key());
            std::lock_guard<std::mutex> lock(mutex);
            registry.erase(inst -> id);
        }
    
        static void bind(Machine* vm) {
            vm_module::register_type<Timer>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(exec, interval, executions)")
                    .require(1, &Machine::is_function)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_number);

                int interval = std::max(1, vm -> get_int(2));
                int executions = std::max(0, vm -> get_int(3));
                std::string env_id = vm -> get_environment_id();
                auto inst = std::make_shared<Instance>();
                inst -> id = next_id.fetch_add(1);
                inst -> env_id = env_id;
                inst -> vm = vm;
                vm -> push(1);
                vm -> set_reference(inst -> ref_key(), -1);
                vm -> pop(1);
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    registry[inst -> id] = inst;
                }
                vm -> create_object(base_name, inst.get());
                auto weak = std::weak_ptr<Instance>(inst);

                Tool::Timer([weak](Tool::Timer* self) {
                    auto inst = weak.lock();
                    if (!inst || inst -> destroyed) {
                        self -> stop();
                        return;
                    }
                    Vital::Engine::Core::get_singleton() -> push_deferred([weak]() {
                        auto inst = weak.lock();
                        if (!inst || inst -> destroyed) return;
                        inst -> vm -> get_reference(inst -> ref_key(), true);
                        inst -> vm -> pcall(0, 0);
                    });
                }, interval, executions);

                if (executions > 0) {
                    // TODO: Needed?? above timer can intenrally handle it probably next to self -> stop();?? // Anisa
                    int cleanup_ms = interval * executions + interval;
                    Tool::Timer([weak](Tool::Timer* self) {
                        auto inst = weak.lock();
                        if (!inst || inst -> destroyed) return;
                        inst -> destroyed = true;
                        Vital::Engine::Core::get_singleton() -> push_deferred([weak]() {
                            cleanup(weak.lock());
                        });
                    }, cleanup_ms, 1);
                }
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, base_name, "destroy", [](auto vm, auto self, auto& id) -> int {
                if (!self || self -> destroyed) { vm -> push_value(false); return 1; }
                self -> destroyed = true;
                auto inst = get_inst(self -> id);
                if (inst) {
                    Vital::Engine::Core::get_singleton() -> push_deferred([inst]() {
                        cleanup(inst);
                    });
                }
                vm -> push_value(true);

                /*
                // TODO: Should release userdata too // Anisa
                vm_module::release_userdata(vm, 1);
                vm -> push_value(true);
                return 1;
                */
                return 1;
            });
        }
    };
}