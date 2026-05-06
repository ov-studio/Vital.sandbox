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
        };
        inline static std::mutex mutex;
        inline static std::unordered_map<int, std::shared_ptr<Instance>> buffer;
        inline static std::atomic<int> next_id { 1 };

        static void clean_instance(std::shared_ptr<Instance> instance) {
            if (!Instance::erase(instance)) return;
            Instance::release(instance);
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Timer>(vm, base_name);

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
                instance -> set_ref(instance -> reference(), 1);
                vm -> pop(1);
                Instance::store(instance);
                vm -> create_object(base_name, instance.get());
                instance -> userdata = vm_module::get_userdata_ptr(vm, -1);

                auto weak = std::weak_ptr<Instance>(instance);
                Tool::Timer::create([weak, executions](Tool::Timer* self, int count) {
                    auto instance = weak.lock();
                    if (!instance || instance -> destroyed) return self -> stop();
                    instance -> vm -> get_reference(instance -> reference(), true);
                    instance -> vm -> push_value(count);
                    instance -> vm -> pcall(1, 0);
                    if ((executions > 0) && (count >= executions)) {
                        instance -> destroyed = true;
                        clean_instance(instance);
                    }
                }, interval, executions);
                return 1;
            });
        }

        static void clean(const std::string& env) {
            vm_module::collect_env<Instance>(mutex, buffer, env, clean_instance);
        }
    };
}
