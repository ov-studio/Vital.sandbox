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
    struct File : Vital::Tool::Module {
        static void bind(void* machine) {
            auto vm = Machine::to_machine(machine);

            API::bind(vm, "file", "exists", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                vm -> push_bool(Vital::Tool::File::exists(to_godot_string(get_directory()), to_godot_string(path)));
                return 1;
            });
        
            API::bind(vm, "file", "size", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                vm -> push_number(static_cast<double>(Vital::Tool::File::size(to_godot_string(get_directory()), to_godot_string(path))));
                return 1;
            });
        
            API::bind(vm, "file", "delete", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                vm -> push_bool(Vital::Tool::File::remove(to_godot_string(get_directory()), to_godot_string(path)));
                return 1;
            });
        
            API::bind(vm, "file", "read", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                auto buffer = Vital::Tool::File::read_text(to_godot_string(get_directory()), to_godot_string(path));
                vm -> push_string(buffer);
                return 1;
            });
        
            API::bind(vm, "file", "write", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(1)) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                auto buffer = vm -> get_string(2);
                vm -> push_bool(Vital::Tool::File::write_text(to_godot_string(get_directory()), to_godot_string(path), buffer));
                return 1;
            });
        
            API::bind(vm, "file", "contents", [](auto* vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                bool directory_search = vm -> is_bool(2) ? vm -> get_bool(2) : false;
                vm -> create_table();
                for (const auto& i : Vital::Tool::File::contents(to_godot_string(get_directory()), to_godot_string(path), directory_search)) {
                    vm -> table_push_string(i);
                }
                return 1;
            });
        }
    };
}