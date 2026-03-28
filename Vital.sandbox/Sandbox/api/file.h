/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: api: file.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: File APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/machine.h>


////////////////////////////////
// Vital: Sandbox: API: File //
////////////////////////////////

namespace Vital::Sandbox::API {
    struct File : vm_module {
        inline static const std::string base_name = "file";

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "exists", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                vm -> push_value(Vital::Tool::File::exists(get_directory(), path));
                return 1;
            });
        
            API::bind(vm, {base_name}, "size", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                vm -> push_value(static_cast<double>(Vital::Tool::File::size(get_directory(), path)));
                return 1;
            });
        
            API::bind(vm, {base_name}, "hash", [](auto vm) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_string(1)) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto mode = vm -> get_string(1);
                auto path = vm -> get_string(2);
                auto hash = Vital::Tool::File::hash(get_directory(), path, mode);
                vm -> push_value(hash);
                return 1;
            });

            API::bind(vm, {base_name}, "delete", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                vm -> push_value(Vital::Tool::File::remove(get_directory(), path));
                return 1;
            });
        
            API::bind(vm, {base_name}, "read", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                auto buffer = Vital::Tool::File::read_text(get_directory(), path);
                vm -> push_value(buffer);
                return 1;
            });
        
            API::bind(vm, {base_name}, "write", [](auto vm) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_string(1)) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                auto buffer = vm -> get_string(2);
                vm -> push_value(Vital::Tool::File::write_text(get_directory(), path, buffer));
                return 1;
            });
        
            API::bind(vm, {base_name}, "contents", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                bool directory_search = vm -> is_bool(2) ? vm -> get_bool(2) : false;
                vm -> create_table();
                for (const auto& i : Vital::Tool::File::contents(get_directory(), path, directory_search)) {
                    vm -> table_push_value(i);
                }
                return 1;
            });
        }
    };
}