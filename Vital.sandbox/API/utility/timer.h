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
            std::string env;
            std::atomic<bool> destroyed { false };
            Machine* vm = nullptr;
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
            if (instance -> vm) {
                vm_module::release_userdata_ptr(instance -> userdata);
                instance -> vm -> del_reference(instance -> reference());
                instance -> vm = nullptr;
            }
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
                auto instance = std::make_shared<Instance>();
                instance -> id = next_id.fetch_add(1);
                instance -> env = vm -> get_environment_id();
                instance -> vm = vm;
                vm -> push(1);
                vm -> set_reference(instance -> reference(), -1);
                vm -> pop(1);
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    buffer[instance -> id] = instance;
                }
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

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, base_name, "destroy", [](auto vm, auto self, auto& id) -> int {
                if (self -> destroyed) { vm -> push_value(false); return 1; }
                self -> destroyed = true;
                auto instance = fetch_instance(self -> id);
                if (instance) clean_instance(instance);
                vm_module::release_userdata(vm, 1);
                vm -> push_value(true);
                return 1;
            });
        }

        static void clean(const std::string& env) {
            vm_module::collect_env(mutex, buffer, env, clean_instance);
        }
    };
}