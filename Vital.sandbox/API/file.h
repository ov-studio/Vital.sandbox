/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: file.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: File APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Manager/public/resource.h>


///////////////////////
// Vital: API: File //
///////////////////////

namespace Vital::Sandbox::API {
    struct File : vm_module {
        inline static const std::string base_name = "file";

        static std::string get_base(Machine* vm, std::string& path) {
            if (!path.empty() && path[0] == ':') {
                const size_t slash = path.find('/');
                if (slash == std::string::npos) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                const std::string target = path.substr(1, slash - 1);
                path = path.substr(slash + 1);
                return Vital::Manager::Resource::get_resource_base(target, true);
            }
            const std::string name = Vital::Manager::Resource::get_resource_from_vm(vm);
            return name.empty() ? get_directory("resources") : Vital::Manager::Resource::get_resource_base(name);
        }

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "exists", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                vm -> push_value(Vital::Tool::File::exists(get_base(vm, path), path));
                return 1;
            });

            API::bind(vm, {base_name}, "size", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                vm -> push_value(static_cast<double>(Vital::Tool::File::size(get_base(vm, path), path)));
                return 1;
            });

            API::bind(vm, {base_name}, "hash", [](auto vm) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_string(1)) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                auto mode = vm -> get_string(2);
                vm -> push_value(Vital::Tool::File::hash(get_base(vm, path), path, mode));
                return 1;
            });

            API::bind(vm, {base_name}, "delete", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                vm -> push_value(Vital::Tool::File::remove(get_base(vm, path), path));
                return 1;
            });

            API::bind(vm, {base_name}, "read", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                vm -> push_value(Vital::Tool::File::read_text(get_base(vm, path), path));
                return 1;
            });

            API::bind(vm, {base_name}, "write", [](auto vm) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_string(1)) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                auto buffer = vm -> get_string(2);
                vm -> push_value(Vital::Tool::File::write_text(get_base(vm, path), path, buffer));
                return 1;
            });

            API::bind(vm, {base_name}, "contents", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                bool directory_search = vm -> is_bool(2) ? vm -> get_bool(2) : false;
                const std::string base = get_base(vm, path);
                vm -> create_table();
                for (const auto& i : Vital::Tool::File::contents(base, path, directory_search)) {
                    vm -> table_push_value(i);
                }
                return 1;
            });
        }
    };
}