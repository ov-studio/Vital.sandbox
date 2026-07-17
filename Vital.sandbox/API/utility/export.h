/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: export.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Export APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Manager/public/resource.h>


/////////////////////////
// Vital: API: Export //
/////////////////////////

namespace Vital::Sandbox::API {
    struct Export : vm_module {
        inline static const std::vector<std::string> base_scope = {"util", "export"};

        inline static std::mutex mutex;
        inline static std::unordered_map<std::string, std::unordered_map<std::string, int>> buffer;


        // Getters //
        static bool register_export(Machine* vm, const std::string& resource, const std::string& name, int reference) {
            std::lock_guard<std::mutex> lock(mutex);
            auto& map = buffer[resource];
            auto it = map.find(name);
            if (it != map.end()) {
                vm -> del_raw_reference(it -> second);
                map.erase(it);
            }
            map[name] = reference;
            return true;
        }
    
        static std::vector<std::string> list_exports(const std::string& resource) {
            std::lock_guard<std::mutex> lock(mutex);
            std::vector<std::string> result;
            auto it = buffer.find(resource);
            if (it == buffer.end()) return result;
            result.reserve(it -> second.size());
            for (const auto& [name, _] : it -> second) result.push_back(name);
            return result;
        }

        static int get_export_ref(const std::string& resource, const std::string& name) {
            std::lock_guard<std::mutex> lock(mutex);
            auto rit = buffer.find(resource);
            if (rit == buffer.end()) return LUA_NOREF;
            auto fit = rit -> second.find(name);
            if (fit == rit -> second.end()) return LUA_NOREF;
            return fit -> second;
        }
    
        static void clean(const std::string& resource) {
            auto vm = Manager::Sandbox::get_singleton() -> get_vm();
            if (!vm) return;

            std::lock_guard<std::mutex> lock(mutex);
            auto it = buffer.find(resource);
            if (it == buffer.end()) return;
            for (auto& [name, ref] : it -> second) vm -> del_raw_reference(ref);
            buffer.erase(it);
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "register", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, exec)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_function);

                const std::string name = vm -> get_string(1);
                const std::string resource = Manager::Resource::get_resource_from_vm(vm);
                // TODO: Improve logs?? Reason: export.register
                if (resource.empty()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: export.register called outside a resource environment"); // TODO: APPLY BASE NAME USING FMT

                register_export(vm, resource, name, vm -> set_raw_reference(2));
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "list", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(resource)")
                    .require(1, &Machine::is_string);

                const std::string resource = vm -> get_string(1);
                if (!Manager::Resource::get_singleton() -> is_running(resource)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: export.list — resource '{}' is not running", resource)); // TODO: APPLY BASE NAME USING FMT

                const auto names = list_exports(resource);
                vm -> create_table();
                for (int i = 0; i < static_cast<int>(names.size()); ++i) {
                    vm -> push_value(names[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            API::bind(vm, base_scope, "call", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(resource, name, ...)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_string);

                const std::string resource = vm -> get_string(1);
                const std::string name = vm -> get_string(2);
                if (!Manager::Resource::get_singleton() -> is_running(resource)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: export.call — resource '{}' is not running", resource));
                int ref = get_export_ref(resource, name);
                if (ref == LUA_NOREF) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: export.call — resource '{}' has no export '{}'", resource, name)); // TODO: APPLY BASE NAME USING FMT

                int nargs = vm -> get_count() - 2;
                vm -> get_raw_reference(ref);
                if (nargs > 0) vm -> rotate(3, 1);
                // TODO: can't use vm->call — it logs+pops on failure; we need to throw instead
                if (lua_pcall(vm -> get_state(), nargs, LUA_MULTRET, 0) != LUA_OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: export.call — error in '{}:{}': {}", resource, name, vm -> get_string(-1))); // TODO: APPLY BASE NAME USING FMT
                return vm -> get_count() - 2;
            });
        }
    };
}