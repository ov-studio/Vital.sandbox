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
            int func_ref = LUA_NOREF;
            Machine* vm = nullptr;
        };
        inline static std::mutex registry_mutex;
        inline static std::unordered_map<int, std::shared_ptr<Instance>> registry;
        inline static std::atomic<int> next_id{1};

        static void cancel_env(const std::string& env_id) {
            std::lock_guard<std::mutex> lock(registry_mutex);
            for (auto& [id, inst] : registry) {
                if (inst -> env_id == env_id) inst -> destroyed = true;
            }
        }

        static std::shared_ptr<Instance> get_inst(int id) {
            std::lock_guard<std::mutex> lock(registry_mutex);
            auto it = registry.find(id);
            return it != registry.end() ? it -> second : nullptr;
        }

        static void cleanup(std::shared_ptr<Instance> inst) {
            if (!inst) return;
            if (inst -> func_ref != LUA_NOREF) {
                luaL_unref(inst -> vm -> get_state(), LUA_REGISTRYINDEX, inst -> func_ref);
                inst -> func_ref = LUA_NOREF;
            }
            std::lock_guard<std::mutex> lock(registry_mutex);
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
                vm -> push(1);
                int func_ref = luaL_ref(vm -> get_state(), LUA_REGISTRYINDEX);
                std::string env_id = vm -> get_environment_id();
                auto inst = std::make_shared<Instance>();
                inst -> id = next_id.fetch_add(1);
                inst -> env_id = env_id;
                inst -> func_ref = func_ref;
                inst -> vm = vm;

                {
                    std::lock_guard<std::mutex> lock(registry_mutex);
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
                        lua_State* L = inst -> vm -> get_state();
                        lua_rawgeti(L, LUA_REGISTRYINDEX, inst -> func_ref);
                        // TODO: No need to catch runtime error?? and function check // Anisa
                        if (lua_isfunction(L, -1)) {
                            if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
                                const char* err = lua_tostring(L, -1);
                                Tool::print("error", fmt::format("Timer: runtime error: {}", err ? err : "?"));
                                lua_pop(L, 1);
                            }
                        } else {
                            lua_pop(L, 1);
                        }
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