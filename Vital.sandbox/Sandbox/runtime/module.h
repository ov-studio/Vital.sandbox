/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: module.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Module
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/runtime/types.h>


/////////////////////
// Vital: Sandbox //
/////////////////////

namespace Vital::Sandbox {
    struct vm_module {
        private:
            using entity_type = std::string;
            using entity_collector = std::function<void(Machine*, int&, bool)>;
            inline static std::unordered_map<entity_type, entity_collector> entity_pool;
        public:
            inline static constexpr bool has_remote = false;
            inline static constexpr bool has_streaming = false;
            static void bind(Machine* vm) {}
            static void methods(Machine* vm) {}
            static void inject(Machine* vm) {}
            static void clean(const std::string& env) {}

            template<typename T>
            static vm_api make_api() {
                Machine::register_environment_cleaner([](const std::string& env) { T::clean(env); });
                return {
                    [](Machine* vm) { T::bind(vm); },
                    [](Machine* vm) { T::inject(vm); }
                };
            }

            template<typename T>
            static void register_type(Machine* vm) {
                vm -> create_metatable(T::base_name);
                vm -> create_table();
                T::methods(vm);
                bind_natives<typename T::Instance>(vm);
                vm -> set_table_field("__index", -2);
                lua_pushcclosure(vm -> get_state(), [](vm_state* state) -> int {
                    auto ud = static_cast<void**>(lua_touserdata(state, 1));
                    release_userdata_ptr(ud);
                    return 0;
                }, 0);
                vm -> set_table_field("__gc", -2);
                vm -> pop(1);

                entity_pool.emplace(T::base_name, [](Machine* vm, int& count, bool streamed) {
                    std::lock_guard<std::mutex> lock(T::registry.mutex);
                    for (auto& [instance_id, instance] : T::registry.buffer) {
                        if (!instance -> is_alive()) continue;
                        #if defined(VSDK_Client)
                        if (streamed && !instance -> is_streamed()) continue;
                        #endif
                        if (instance -> userdata) {
                            instance -> get_reference(instance -> self_reference(), true);
                            vm -> set_table_field(++count, -2);
                        }
                    }
                });
            }

            template<typename T>
            static void bind_method(Machine* vm, const std::string& name, std::function<int(Machine*, std::shared_ptr<T>, const std::string&)> exec) {
                // TODO: These needs to be freed when changing server freeing core // Anisa
                auto heap_exec = new std::function<int(Machine*, std::shared_ptr<T>, const std::string&)>(std::move(exec));
                auto heap_type = new std::string(T::Owner::base_name);
                auto heap_id   = new std::string(std::string(T::Owner::base_name) + ":" + name);
                lua_pushlightuserdata(vm -> get_state(), heap_exec);
                lua_pushlightuserdata(vm -> get_state(), heap_type);
                lua_pushlightuserdata(vm -> get_state(), heap_id);
                lua_pushcclosure(vm -> get_state(), [](vm_state* state) -> int {
                    auto fn   = static_cast<std::function<int(Machine*, std::shared_ptr<T>, const std::string&)>*>(lua_touserdata(state, lua_upvalueindex(1)));
                    auto type = static_cast<std::string*>(lua_touserdata(state, lua_upvalueindex(2)));
                    auto id   = static_cast<std::string*>(lua_touserdata(state, lua_upvalueindex(3)));
                    auto vm   = Machine::fetch_machine(state);
                    return vm -> execute([&]() -> int {
                        auto ud = static_cast<void**>(luaL_checkudata(state, 1, type -> c_str()));
                        auto throw_destroyed = [&]() { throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: `<{}>` instance was destroyed", *type)); };
                        if (!ud || !*ud) throw_destroyed();
                        auto self = T::find_unlocked(static_cast<T*>(*ud) -> id);
                        if (!self) throw_destroyed();
                        return (*fn)(vm, self, *id);
                    });
                }, 3);
                lua_setfield(vm -> get_state(), -2, name.c_str());
            }

            template<typename TInstance>
            static void bind_natives(Machine* vm) {
                using TOwner = typename TInstance::Owner;

                bind_method<TInstance>(vm, "is_type", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(type_name)")
                        .require(2, &Machine::is_string);

                    vm -> push_value(std::string(TOwner::base_name) == vm -> get_string(2));
                    return 1;
                });

                #if defined(VSDK_Client)
                if constexpr (TOwner::has_remote) {
                    bind_method<TInstance>(vm, "is_remote", [](auto vm, auto self, auto& id) -> int {
                        vm -> push_value(self -> is_remote());
                        return 1;
                    });
                }

                if constexpr (TOwner::has_streaming) {
                    bind_method<TInstance>(vm, "is_streamed", [](auto vm, auto self, auto& id) -> int {
                        vm -> push_value(self -> is_streamed());
                        return 1;
                    });
                }
                #endif

                bind_method<TInstance>(vm, "get_type", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(std::string(TOwner::base_name).empty() ? false : TOwner::base_name);
                    return 1;
                });

                bind_method<TInstance>(vm, "destroy", [](auto vm, auto self, auto& id) -> int {
                    #if defined(VSDK_Client)
                    if constexpr (TOwner::has_remote) {
                        if (self -> is_remote()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: remote entities cannot be destroyed by the client");
                    }
                    #endif
                    return TInstance::destroy(vm);
                });
            }

            template<typename T>
            static bool is_userdata(Machine* vm, int index = 1) {
                auto ud = static_cast<void**>(luaL_testudata(vm -> get_state(), index, T::Owner::base_name.c_str()));
                if (!ud || !*ud) return false;
                return T::find_unlocked(static_cast<T*>(*ud) -> id) != nullptr;
            }

            template<typename T = void>
            static std::string get_userdata_type(Machine* vm, int index = 1) {
                auto state = vm -> get_state();
                if (!lua_isuserdata(state, index) || !lua_getmetatable(state, index)) return "";
                lua_getfield(state, -1, "__name");
                const char* name = lua_tostring(state, -1);
                lua_pop(state, 2);
                return name ? name : "";
            }

            template<typename T>
            static std::shared_ptr<T> get_userdata_object(Machine* vm, int index = 1) {
                auto ud = get_userdata_ptr(vm, index);
                if (!ud || !*ud) return nullptr;
                return T::find_unlocked(static_cast<T*>(*ud) -> id);
            }

            template<typename T = void>
            static void** get_userdata_ptr(Machine* vm, int index = 1) {
                return static_cast<void**>(lua_touserdata(vm -> get_state(), index));
            }

            template<typename T = void>
            static void release_userdata(Machine* vm, int index = 1) {
                auto ud = get_userdata_ptr(vm, index);
                release_userdata_ptr(ud);
            }

            template<typename T = void>
            static void release_userdata_ptr(void**& userdata) {
                if (!userdata) return;
                *userdata = nullptr;
                userdata  = nullptr;
            }

            static void collect_entities(Machine* vm, const std::string& type, int& count, bool streamed = false) {
                auto it = entity_pool.find(type);
                if (it != entity_pool.end()) it -> second(vm, count, streamed);
            }

            template<typename TInstance>
            static void collect_env(vm_registry<TInstance>& registry, const std::string& env, std::function<void(std::shared_ptr<TInstance>)> clean) {
                std::vector<std::shared_ptr<TInstance>> to_clean;
                {
                    std::lock_guard<std::mutex> lock(registry.mutex);
                    for (auto& [id, instance] : registry.buffer) {
                        if (instance -> env.empty()) continue;
                        if (instance -> env != env) continue;
                        instance -> destroyed = true;
                        to_clean.push_back(instance);
                    }
                }
                for (auto& instance : to_clean) clean(instance);
            }
    };
}
