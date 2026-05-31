/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: index.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/machine.h>
#include <Vital.sandbox/Tool/index.h>
#include <Vital.sandbox/Tool/log.h>
#include <Vital.sandbox/Tool/yaml.h>
#include <Vital.sandbox/Tool/stack.h>
#include <Vital.sandbox/Tool/thread.h>
#include <Vital.sandbox/Tool/timer.h>
#include <Vital.sandbox/Tool/file.h>
#include <Vital.sandbox/Tool/format.h>
#include <Vital.sandbox/Tool/event.h>
#include <Vital.sandbox/Tool/database.h>
#include <Vital.sandbox/Tool/http.h>
#include <Vital.sandbox/Tool/inspect.h>
#include <Vital.sandbox/Tool/crypto.h>
#include <Vital.sandbox/Tool/shrinker.h>
#include <Vital.sandbox/Vendor/lua/lua.hpp>


/////////////////////
// Vital: Sandbox //
/////////////////////

namespace Vital::Sandbox {
    class Machine;
    using vm_state = lua_State;
    using vm_exec = lua_CFunction;
    using vm_machines = std::unordered_map<vm_state*, Machine*>;
    using vm_refs = std::unordered_map<std::string, int>;
    using vm_bind = std::function<int(Machine*, const std::string&)>;
    using vm_method = std::function<int(Machine*, void*, const std::string&)>;

    struct vm_api {
        std::function<void(Machine*)> bind;
        std::function<void(Machine*)> inject;
    };
    using vm_apis = std::vector<vm_api>;
    using vm_env_cleaner = std::function<void(const std::string&)>;
    using vm_env_cleaners = std::vector<vm_env_cleaner>;

    struct vm_error {
        std::string detail;
        std::string partial;
        vm_error(const std::string& detail, const std::string& partial) : detail(detail), partial(partial) {}
    };

    struct vm_args {
        private:
            inline void throw_error(int index, const std::string& reason = "") const {
                const std::string arg = (index - 1) < (int)arguments.size() ? arguments[index - 1] : std::to_string(index);
                const std::string partial = fmt::format("bad argument #{} '{}' {}", index, arg, reason.empty() ? "" : fmt::format("({})", reason));
                const std::string detail = fmt::format("invalid argument\n> Syntax: `{}`\n> Reason: {}", syntax, partial);
                throw vm_error(detail, partial);
            }
        public:
            Machine* vm;
            std::string syntax;
            std::vector<std::string> arguments;

            inline vm_args(Machine* vm, const std::string& syntax) : vm_args(vm, syntax, "") {}
            inline vm_args(Machine* vm, const std::string& id, const std::string& args) : vm(vm), syntax(id + args) {
                auto start = syntax.find('(');
                auto end = syntax.find(')');
                if (start == std::string::npos || end == std::string::npos) return;
                std::stringstream ss(syntax.substr(start + 1, end - start - 1));
                std::string token;
                while (std::getline(ss, token, ',')) {
                    token.erase(0, token.find_first_not_of(" \t"));
                    token.erase(token.find_last_not_of(" \t") + 1);
                    if (!token.empty()) arguments.push_back(token);
                }
            }

            template<typename F>
            inline vm_args& require(int index, F&& check) {
                if ((vm -> get_count() < index) || !std::invoke(std::forward<F>(check), vm, index)) throw_error(index);
                return *this;
            }

            template<typename F>
            inline vm_args& optional(int index, F&& check) {
                if ((vm -> get_count() >= index) && !vm -> is_nil(index) && !std::invoke(std::forward<F>(check), vm, index)) throw_error(index);
                return *this;
            }

            template<typename F>
            inline vm_args& validate(int index, F&& check, const std::string& reason = "out of range") {
                if (!std::invoke(std::forward<F>(check), vm, index)) throw_error(index, reason);
                return *this;
            }

            template<typename E>
            inline vm_args& validate_enum(int index, E min, E max) {
                return validate(index, [min, max](Machine* vm, int index) {
                    auto value = vm -> get_int(index);
                    return value >= static_cast<int>(min) && value <= static_cast<int>(max);
                }, fmt::format("out of range [{} - {}]", static_cast<int>(min), static_cast<int>(max)));
            }
    };

    template<typename T>
    struct vm_registry {
        std::mutex mutex;
        std::unordered_map<int, std::shared_ptr<T>> buffer;
        std::atomic<int> next_id { 1 };
    };

    struct vm_module {
        private:
            using entity_type = std::string;
            using entity_collector = std::function<void(Machine*, int&, bool)>;
            inline static std::unordered_map<entity_type, entity_collector> entity_pool;
        public:
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
            static void register_type(Machine* vm, const std::string& type_name) {
                vm -> create_metatable(type_name);
                vm -> create_table();
                T::methods(vm);
                bind_natives<T, typename T::Instance>(vm, type_name);
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
                        #if defined(Vital_SDK_Client)
                        if (streamed && !instance -> is_streamed()) continue;
                        #endif
                        if (instance -> userdata) instance -> vm -> get_reference(instance -> self_reference(), true);
                        else instance -> bind(vm, T::base_name);
                        vm -> set_table_field(++count, -2);
                    }
                });
            }

            template<typename T>
            static void bind_method(Machine* vm, const std::string& type_name, const std::string& name, std::function<int(Machine*, std::shared_ptr<T>, const std::string&)> exec) {
                // TODO: These needs to be freed when changing server freeing core // Anisa
                auto heap_exec = new std::function<int(Machine*, std::shared_ptr<T>, const std::string&)>(std::move(exec));
                auto heap_type = new std::string(type_name);
                auto heap_id = new std::string(type_name + ":" + name);
                lua_pushlightuserdata(vm -> get_state(), heap_exec);
                lua_pushlightuserdata(vm -> get_state(), heap_type);
                lua_pushlightuserdata(vm -> get_state(), heap_id);
                lua_pushcclosure(vm -> get_state(), [](vm_state* state) -> int {
                    auto fn = static_cast<std::function<int(Machine*, std::shared_ptr<T>, const std::string&)>*>(lua_touserdata(state, lua_upvalueindex(1)));
                    auto type = static_cast<std::string*>(lua_touserdata(state, lua_upvalueindex(2)));
                    auto id = static_cast<std::string*>(lua_touserdata(state, lua_upvalueindex(3)));
                    auto vm = Machine::fetch_machine(state);
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

            template<typename TOwner, typename TInstance>
            static void bind_natives(Machine* vm, const std::string& type_name) {
                bind_method<TInstance>(vm, type_name, "is_type", [type_name](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(type_name)")
                        .require(2, &Machine::is_string);
                        
                    vm -> push_value(type_name == vm -> get_string(2));
                    return 1;
                });

                #if defined(Vital_SDK_Client)
                bind_method<TInstance>(vm, type_name, "is_remote", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> is_remote());
                    return 1;
                });

                bind_method<TInstance>(vm, type_name, "is_streamed", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> is_streamed());
                    return 1;
                });
                #endif

                bind_method<TInstance>(vm, type_name, "get_type", [type_name](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(type_name.empty() ? false : type_name);
                    return 1;
                });

                bind_method<TInstance>(vm, type_name, "destroy", [](auto vm, auto self, auto& id) -> int {
                    #if defined(Vital_SDK_Client)
                    if (self -> is_remote()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: remote entities cannot be destroyed by the client");
                    #endif
                    return TInstance::destroy(vm);
                });
            }

            template<typename T>
            static bool is_userdata(Machine* vm, const std::string& type_name, int index = 1) {
                auto ud = static_cast<void**>(luaL_testudata(vm -> get_state(), index, type_name.c_str()));
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

    template<typename Derived>
    struct vm_instance : std::enable_shared_from_this<Derived> {
        private:
            std::vector<std::string> refs;
        public:
            int id {};
            std::string env;
            std::atomic<bool> destroyed { false };
            Machine* vm = nullptr;
            void** userdata = nullptr;
            std::string reference() const { return fmt::format("vm_instance:{}:{}", Derived::Owner::base_name, id); }
            std::string self_reference() const { return fmt::format("vm_instance:{}:{}:self", Derived::Owner::base_name, id); }

            bool is_alive() const { 
                return true; 
            }

            bool is_streamed() const { 
                return true; 
            }

            bool is_remote() const { 
                return env.empty(); 
            }

            void set_ref(const std::string& ref, int index) {
                vm -> set_reference(ref, index);
                refs.push_back(ref);
            }

            bool store() {
                return Derived::store(static_cast<Derived*>(this) -> shared_from_this());
            }

            bool bind(Machine* vm, const std::string& type_name, int index = -1) {
                return Derived::bind(static_cast<Derived*>(this) -> shared_from_this(), vm, type_name, index);
            }

            bool erase() {
                return Derived::erase(static_cast<Derived*>(this) -> shared_from_this());
            }

            bool erase_unlocked() {
                return Derived::erase_unlocked(static_cast<Derived*>(this) -> shared_from_this());
            }

            void clean() {
                auto instance = static_cast<Derived*>(this) -> shared_from_this();
                if (!Derived::erase(instance)) return;
                Derived::release(instance);
            }

            bool release() {
                return Derived::release(static_cast<Derived*>(this) -> shared_from_this());
            }

            static std::shared_ptr<Derived> find(int id) {
                std::lock_guard<std::mutex> lock(Derived::Owner::registry.mutex);
                return find_unlocked(id);
            }

            static std::shared_ptr<Derived> find_unlocked(int id) {
                auto it = Derived::Owner::registry.buffer.find(id);
                if (it == Derived::Owner::registry.buffer.end() || it -> second -> destroyed || !it -> second -> is_alive()) return nullptr;
                return it -> second;
            }

            static bool store(std::shared_ptr<Derived> instance) {
                std::lock_guard<std::mutex> lock(Derived::Owner::registry.mutex);
                Derived::Owner::registry.buffer[instance -> id] = instance;
                return true;
            }

            static bool bind(std::shared_ptr<Derived> instance, Machine* vm, const std::string& type_name, int index = -1) {
                vm -> create_object(type_name, instance.get());
                instance -> userdata = vm_module::get_userdata_ptr(vm, index);
                instance -> set_ref(instance -> self_reference(), index);
                return true;
            }

            static bool erase(std::shared_ptr<Derived> instance) {
                std::lock_guard<std::mutex> lock(Derived::Owner::registry.mutex);
                return erase_unlocked(instance);
            }

            static bool erase_unlocked(std::shared_ptr<Derived> instance) {
                auto it = Derived::Owner::registry.buffer.find(instance -> id);
                if (it == Derived::Owner::registry.buffer.end()) return false;
                Derived::Owner::registry.buffer.erase(it);
                instance -> destroyed = true;
                return true;
            }
            

            static bool release(std::shared_ptr<Derived> instance) {
                vm_module::release_userdata_ptr(instance -> userdata);
                if (instance -> vm) {
                    for (auto& ref : instance -> refs) instance -> vm -> del_reference(ref);
                    instance -> refs.clear();
                }
                instance -> vm = nullptr;
                return true;
            }

            static std::shared_ptr<Derived> init(Machine* vm, bool remote = false) {
                auto instance = std::make_shared<Derived>();
                instance -> id = Derived::Owner::registry.next_id.fetch_add(1);
                instance -> vm = remote ? Manager::Sandbox::get_singleton() -> get_vm() -> get_root() : vm -> get_root();
                if (!remote) instance -> env = vm -> get_environment_id();
                return instance;
            }
    
            static std::shared_ptr<Derived> make(Machine* vm) {
                auto instance = Derived::init(vm);
                Derived::store(instance);
                Derived::bind(instance, vm, Derived::Owner::base_name);
                return instance;
            }

            static int destroy(Machine* vm) {
                auto ud = vm_module::get_userdata_ptr(vm, 1);
                if (ud && *ud) {
                    auto instance = Derived::find_unlocked(static_cast<Derived*>(*ud) -> id);
                    if (instance) instance -> clean();
                }
                vm_module::release_userdata(vm, 1);
                vm -> push_value(true);
                return 1;
            }

            static void collect_env(const std::string& env) {
                vm_module::collect_env(Derived::Owner::registry, env, std::function<void(std::shared_ptr<Derived>)>([](std::shared_ptr<Derived> instance) {
                    instance -> clean();
                }));
            }
    };

    namespace API {
        extern void log(const std::string& mode, const std::string& message);
        extern void bind(Machine* vm, const std::vector<std::string>& scope, const std::string& name, vm_bind exec);
    }
}