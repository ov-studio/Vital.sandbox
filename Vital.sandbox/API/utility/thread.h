/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: thread.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Thread APIs (replaces thread.lua)
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/API/utility/timer.h>
#include <Vital.sandbox/Engine/public/core.h>


/////////////////////////
// Vital: API: Thread //
/////////////////////////

namespace Vital::Sandbox::API {
    struct Thread : vm_module {
        inline static const std::string base_name = "thread";

        struct Instance {
            int              id;
            std::string      env_id;
            Machine*         vm_thread  = nullptr; // the Lua coroutine Machine
            int              func_ref   = LUA_NOREF;
            Machine*         owner_vm   = nullptr;
            std::atomic<bool> destroyed{false};
            // sleep state
            bool             sleeping   = false;
            int              sleep_timer_id = -1;
        };

        inline static std::mutex                                   registry_mutex;
        inline static std::unordered_map<int, std::shared_ptr<Instance>> registry;
        inline static std::atomic<int>                             next_id{1};

        static void cancel_env(const std::string& env_id) {
            std::lock_guard<std::mutex> lock(registry_mutex);
            for (auto& [id, inst] : registry) {
                if (inst->env_id == env_id) {
                    inst->destroyed = true;
                    // Also kill any sleep timer
                    if (inst->sleep_timer_id != -1) {
                        auto it = Timer::registry.find(inst->sleep_timer_id);
                        if (it != Timer::registry.end()) it->second->destroyed = true;
                        inst->sleep_timer_id = -1;
                    }
                }
            }
        }

        static void bind(Machine* vm) {

            // thread:create(exec)
            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(exec)")
                    .require(1, &Machine::is_function);

                std::string env_id = vm -> get_environment_id();

                auto inst       = std::make_shared<Instance>();
                inst->id        = next_id.fetch_add(1);
                inst->env_id    = env_id;
                inst->owner_vm  = vm;

                // Create Lua coroutine
                Machine* thread_vm = vm -> create_thread();
                inst->vm_thread    = thread_vm;

                // Move the function into the coroutine
                vm -> push(1);
                vm -> move(thread_vm, 1);
                inst->func_ref = luaL_ref(thread_vm -> get_state(), LUA_REGISTRYINDEX);

                {
                    std::lock_guard<std::mutex> lock(registry_mutex);
                    registry[inst->id] = inst;
                }

                vm -> push_number(inst->id);
                return 1;
            });

            // thread:resume(thread_id)
            API::bind(vm, {base_name}, "resume", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(thread_id)")
                    .require(1, &Machine::is_number);

                int inst_id = vm -> get_int(1);
                std::shared_ptr<Instance> inst;
                {
                    std::lock_guard<std::mutex> lock(registry_mutex);
                    auto it = registry.find(inst_id);
                    if (it == registry.end()) { vm -> push_value(false); return 1; }
                    inst = it->second;
                }
                if (inst->destroyed || !inst->vm_thread) { vm -> push_value(false); return 1; }

                // Push the function and resume
                lua_rawgeti(inst->vm_thread->get_state(), LUA_REGISTRYINDEX, inst->func_ref);
                inst->vm_thread->resume(1);

                // Clean up if dead
                if (inst->vm_thread->get_state() &&
                    lua_status(inst->vm_thread->get_state()) != LUA_YIELD) {
                    destroy_inst(inst);
                }
                vm -> push_value(true);
                return 1;
            });

            // thread:destroy(thread_id)
            API::bind(vm, {base_name}, "destroy", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(thread_id)")
                    .require(1, &Machine::is_number);

                int inst_id = vm -> get_int(1);
                std::shared_ptr<Instance> inst;
                {
                    std::lock_guard<std::mutex> lock(registry_mutex);
                    auto it = registry.find(inst_id);
                    if (it == registry.end()) { vm -> push_value(false); return 1; }
                    inst = it->second;
                }
                destroy_inst(inst);
                vm -> push_value(true);
                return 1;
            });

            // thread:is_instance(thread_id)
            API::bind(vm, {base_name}, "is_instance", [](auto vm, auto& id) -> int {
                if (!vm -> is_number(1)) { vm -> push_value(false); return 1; }
                int inst_id = vm -> get_int(1);
                std::lock_guard<std::mutex> lock(registry_mutex);
                auto it = registry.find(inst_id);
                vm -> push_value(it != registry.end() && !it->second->destroyed);
                return 1;
            });

            // thread:sleep(thread_id, duration)  — called from within the thread itself
            API::bind(vm, {base_name}, "sleep", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(thread_id, duration)")
                    .require(1, &Machine::is_number)
                    .require(2, &Machine::is_number);

                int inst_id  = vm -> get_int(1);
                int duration = std::max(0, vm -> get_int(2));

                std::shared_ptr<Instance> inst;
                {
                    std::lock_guard<std::mutex> lock(registry_mutex);
                    auto it = registry.find(inst_id);
                    if (it == registry.end()) { vm -> push_value(false); return 1; }
                    inst = it->second;
                }
                if (inst->destroyed || inst->sleeping) { vm -> push_value(false); return 1; }

                inst->sleeping = true;
                auto weak = std::weak_ptr<Instance>(inst);

                // Schedule a one-shot timer to resume after duration
                Tool::Timer([weak](Tool::Timer* self) {
                    Vital::Engine::Core::get_singleton()->push_deferred([weak]() {
                        auto inst = weak.lock();
                        if (!inst || inst->destroyed) return;
                        inst->sleeping = false;
                        inst->sleep_timer_id = -1;
                        if (!inst->vm_thread) return;
                        inst->vm_thread->resume(0);
                        // Check if dead after resume
                        if (lua_status(inst->vm_thread->get_state()) != LUA_YIELD) {
                            destroy_inst(inst);
                        }
                    });
                }, duration, 1);

                // Yield the coroutine
                vm -> pause();
                return 0;
            });
        }

        static void destroy_inst(std::shared_ptr<Instance>& inst) {
            if (!inst || inst->destroyed) return;
            inst->destroyed = true;
            if (inst->func_ref != LUA_NOREF && inst->vm_thread) {
                luaL_unref(inst->vm_thread->get_state(), LUA_REGISTRYINDEX, inst->func_ref);
                inst->func_ref = LUA_NOREF;
            }
            inst->vm_thread = nullptr;
            std::lock_guard<std::mutex> lock(registry_mutex);
            registry.erase(inst->id);
        }
    };
}