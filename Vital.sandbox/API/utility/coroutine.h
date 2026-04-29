/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: coroutine.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Coroutine APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/core.h>


////////////////////////////
// Vital: API: Coroutine //
////////////////////////////

namespace Vital::Sandbox::API {
    struct Coroutine : vm_module {
        inline static const std::string base_name = "coroutine";

        struct SleepEntry {
            Machine*          vm;
            std::string       env_id;
            std::atomic<bool> cancelled{false};
        };

        inline static std::mutex                                        sleep_mutex;
        inline static std::vector<std::shared_ptr<SleepEntry>>          sleep_registry;

        static void cancel_env(const std::string& env_id) {
            std::lock_guard<std::mutex> lock(sleep_mutex);
            for (auto& entry : sleep_registry) {
                if (entry->env_id == env_id) entry->cancelled = true;
            }
        }

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(handler)")
                    .require(1, &Machine::is_function);

                auto thread = vm -> create_thread();
                vm -> push(1);
                vm -> move(thread, 1);
                return 1;
            });

            API::bind(vm, {base_name}, "resume", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(thread)")
                    .require(1, &Machine::is_thread);

                auto thread    = vm -> get_thread(1);
                auto thread_vm = Machine::fetch_machine(thread);
                if (!thread_vm -> is_virtual())
                    throw Tool::Log::fetch("invalid-thread", Tool::Log::Type::error);
                thread_vm -> resume();
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "pause", [](auto vm, auto& id) -> int {
                if (!vm -> is_virtual())
                    throw Tool::Log::fetch("invalid-thread", Tool::Log::Type::error);
                vm -> pause();
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "sleep", [](auto vm, auto& id) -> int {
                if (!vm -> is_virtual())
                    throw Tool::Log::fetch("invalid-thread", Tool::Log::Type::error);
                vm_args(vm, id, "(duration)")
                    .require(1, &Machine::is_number);

                int duration = vm -> get_int(1);
                std::string env_id = vm -> get_environment_id();

                // Track this sleep so cancel_env can kill it
                auto entry = std::make_shared<SleepEntry>();
                entry->vm     = vm;
                entry->env_id = env_id;

                {
                    std::lock_guard<std::mutex> lock(sleep_mutex);
                    sleep_registry.push_back(entry);
                }

                auto weak = std::weak_ptr<SleepEntry>(entry);

                Tool::Timer([weak](Tool::Timer* self) {
                    Vital::Engine::Core::get_singleton()->push_deferred([weak]() {
                        auto entry = weak.lock();
                        if (!entry) return;

                        // Remove from registry
                        {
                            std::lock_guard<std::mutex> lock(sleep_mutex);
                            auto& reg = sleep_registry;
                            reg.erase(
                                std::remove_if(reg.begin(), reg.end(),
                                    [&](const std::shared_ptr<SleepEntry>& e) {
                                        return e.get() == entry.get();
                                    }),
                                reg.end()
                            );
                        }

                        // Only resume if not cancelled
                        if (entry->cancelled) return;
                        entry->vm -> push_value(true);
                        entry->vm -> resume(1);
                    });
                }, duration, 1);

                vm -> pause();
                return 0;
            });
        }
    };
}