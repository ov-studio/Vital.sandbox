/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: exports.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Exports API
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Manager/public/resource.h>


//////////////////////////
// Vital: API: Exports //
//////////////////////////

// TODO: Improve

namespace Vital::Sandbox::API {
    struct Exports : vm_module {
        inline static const std::string base_name = "exports";

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "register", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, fn)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_function);

                const std::string fn_name = vm -> get_string(1);
                const std::string resource = Manager::Resource::get_resource_from_vm(vm);
                if (resource.empty()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: exports.register called outside a resource environment");

                lua_pushvalue(vm -> get_state(), 2);
                int ref = luaL_ref(vm -> get_state(), LUA_REGISTRYINDEX);
                Manager::Sandbox::get_singleton() -> export_add(resource, fn_name, ref);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "call", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(resource, name, ...)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_string);

                const std::string resource = vm -> get_string(1);
                const std::string fn_name  = vm -> get_string(2);
                auto* L = vm -> get_state();
                if (!Manager::Resource::get_singleton() -> is_running(resource)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: exports.call — resource '{}' is not running", resource));

                int ref = Manager::Sandbox::get_singleton() -> export_get_ref(resource, fn_name);
                if (ref == LUA_NOREF) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: exports.call — resource '{}' has no export '{}'", resource, fn_name));
                int nargs = lua_gettop(L) - 2;
                lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
                if (nargs > 0) lua_rotate(L, 3, 1);
                if (lua_pcall(L, nargs, LUA_MULTRET, 0) != LUA_OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: exports.call — error in '{}:{}': {}", resource, fn_name, lua_tostring(L, -1)));
                return lua_gettop(L) - 2;
            });
        }
    };
}