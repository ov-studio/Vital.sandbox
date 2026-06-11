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

        struct Instance : vm_instance<Instance> {
            using Owner = Timer;
            Tool::Timer* timer = nullptr;

            bool is_alive() const { 
                return timer ? true : false; 
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                Tool::Timer* __timer = nullptr;
                {
                    std::lock_guard<std::mutex> lock(registry.mutex);
                    __timer = instance -> timer;
                    instance -> timer = nullptr;
                }
                if (__timer && Tool::Timer::valid(__timer)) __timer -> stop();
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static void bind(Machine* vm) {
            vm_module::register_type<Timer>(vm);

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(exec, interval, executions)")
                    .require(1, &Machine::is_function)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_number)
                    .validate(2, [](auto vm, int index) { return vm -> get_int(index) >= 0; }, "expected >= 0")
                    .validate(3, [](auto vm, int index) { return vm -> get_int(index) >= 0; }, "expected >= 0");

                int interval = std::max(1, vm -> get_int(2));
                int executions = std::max(0, vm -> get_int(3));
                auto instance = Instance::init(vm);
                instance -> set_reference(instance -> reference(), 1);
                vm -> pop(1);
                instance -> store();

                auto weak = std::weak_ptr<Instance>(instance);
                auto timer = Tool::Timer::create([weak, executions](Tool::Timer*, int count) {
                    int captured_count = count;
                    bool captured_stop = (executions > 0) && (count >= executions);
                    Machine::enqueue([weak, captured_count, captured_stop]() {
                        auto instance = weak.lock();
                        if (!Instance::find_unlocked(instance)) return;
                        instance -> get_reference(instance -> reference(), true);
                        instance -> vm -> push_value(captured_count);
                        instance -> vm -> pcall(1, 0);
                        if (captured_stop) {
                            {
                                std::lock_guard<std::mutex> lock(registry.mutex);
                                instance -> timer = nullptr;
                            }
                            instance -> clean();
                        }
                    });
                }, interval, executions);

                {
                    std::lock_guard<std::mutex> lock(registry.mutex);
                    instance -> timer = timer;
                }
                return 1;
            });
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}