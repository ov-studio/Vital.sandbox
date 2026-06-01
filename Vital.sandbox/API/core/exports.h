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

                int ref = vm -> set_raw_reference(2);
                Manager::Sandbox::get_singleton() -> export_add(resource, fn_name, ref);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "call", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(resource, name, ...)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_string);

                const std::string resource = vm -> get_string(1);
                const std::string fn_name = vm -> get_string(2);
                if (!Manager::Resource::get_singleton() -> is_running(resource)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: exports.call — resource '{}' is not running", resource));
                int ref = Manager::Sandbox::get_singleton() -> export_get_ref(resource, fn_name);
                if (ref == LUA_NOREF) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: exports.call — resource '{}' has no export '{}'", resource, fn_name));

                int nargs = vm -> get_count() - 2;
                vm -> get_raw_reference(ref);
                if (nargs > 0) vm -> rotate(3, 1);
                // can't use vm->pcall — it logs+pops on failure; we need to throw instead
                if (lua_pcall(vm -> get_state(), nargs, LUA_MULTRET, 0) != LUA_OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: exports.call — error in '{}:{}': {}", resource, fn_name, vm -> get_string(-1)));
                return vm -> get_count() - 2;
            });
        }
    };
}