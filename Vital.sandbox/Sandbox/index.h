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
#include <Vital.sandbox/Tool/event.h>
#include <Vital.sandbox/Tool/database.h>
#include <Vital.sandbox/Tool/rest.h>
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

    struct vm_args {
        private:
            inline void throw_error(int index, const std::string& reason = "") const {
                const std::string arg = (index - 1) < (int)arg_names.size() ? arg_names[index - 1] : std::to_string(index);
                std::string detail = fmt::format("\n> Syntax: `{}`", syntax);
                detail += fmt::format("\n> Reason: bad argument #{} '{}' {}", index, arg, reason.empty() ? "" : fmt::format("({})", reason));
                throw Tool::Log::fetch("invalid-arguments", Tool::Log::Type::Error, detail);
            }
        public:
            Machine* vm;
            std::string syntax;
            std::vector<std::string> arg_names;
        
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
                    if (!token.empty()) arg_names.push_back(token);
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
    };

    struct vm_module {
        static void bind(Machine* vm) {}
        static void methods(Machine* vm) {}
        static void inject(Machine* vm) {}
    
        template<typename T>
        static vm_api make_api() {
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
            bind_natives<T>(vm, type_name);
            vm -> set_table_field("__index", -2);
            lua_pushcclosure(vm -> get_state(), [](vm_state* state) -> int {
                void** ud = static_cast<void**>(lua_touserdata(state, 1));
                if (ud) *ud = nullptr;
                return 0;
            }, 0);
            vm -> set_table_field("__gc", -2);
            vm -> pop(1);
        }
    
        template<typename T>
        static void bind_method(Machine* vm, const std::string& type_name, const std::string& name, std::function<int(Machine*, T*, const std::string&)> exec) {
            auto heap_exec = new std::function<int(Machine*, T*, const std::string&)>(std::move(exec));
            auto heap_type = new std::string(type_name);
            auto heap_id = new std::string(type_name + ":" + name);
            lua_pushlightuserdata(vm -> get_state(), heap_exec);
            lua_pushlightuserdata(vm -> get_state(), heap_type);
            lua_pushlightuserdata(vm -> get_state(), heap_id);
            lua_pushcclosure(vm -> get_state(), [](vm_state* state) -> int {
                auto fn = static_cast<std::function<int(Machine*, T*, const std::string&)>*>(lua_touserdata(state, lua_upvalueindex(1)));
                auto type = static_cast<std::string*>(lua_touserdata(state, lua_upvalueindex(2)));
                auto id = static_cast<std::string*>(lua_touserdata(state, lua_upvalueindex(3)));
                auto vm = Machine::fetch_machine(state);
                return vm -> execute([&]() -> int {
                    void** ud = static_cast<void**>(luaL_checkudata(state, 1, type -> c_str()));
                    if (!ud || !*ud) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, fmt::format("Invalid {} instance", *type));
                    return (*fn)(vm, static_cast<T*>(*ud), *id);
                });
            }, 3);
            lua_setfield(vm -> get_state(), -2, name.c_str());
        }
    
        template<typename T>
        static void bind_natives(Machine* vm, const std::string& type_name) {
            bind_method<T>(vm, type_name, "is_type", [type_name](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(type_name)")
                    .require(2, &Machine::is_string);

                vm -> push_value(type_name == vm -> get_string(2));
                return 1;
            });
    
            bind_method<T>(vm, type_name, "get_type", [type_name](auto vm, auto self, auto& id) -> int {
                vm -> push_value(type_name.empty() ? false : type_name);
                return 1;
            });
        }
    
        template<typename T = void>
        static bool is_userdata(Machine* vm, const std::string& type_name, int index = 1) {
            void** ud = static_cast<void**>(luaL_testudata(vm -> get_state(), index, type_name.c_str()));
            return ud && *ud;
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
    
        template<typename T = void>
        static void release_userdata(Machine* vm, int index = 1) {
            void** ud = static_cast<void**>(lua_touserdata(vm -> get_state(), index));
            if (ud) *ud = nullptr;
        }
    };

    namespace API {
        extern void log(const std::string& type, const std::string& message);
        extern void bind(Machine* vm, const std::vector<std::string>& scope, const std::string& name, vm_bind exec);
    }
}