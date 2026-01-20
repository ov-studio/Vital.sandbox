/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: file.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: File APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/index.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Sandbox::Lua::API {
    class File : public Vital::Tool::Module {
        public:
            inline static void bind(void* instance) {
                auto vm = Vital::Sandbox::Lua::Machine::to_vm(instance);

                API::bind(vm, "file", "exists", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::Machine::fetch_vm(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto path = vm -> get_string(1);
                        vm -> push_bool(Vital::Tool::File::exists(to_godot_string(get_directory()), to_godot_string(path)));
                        return 1;
                    });
                });
            
                API::bind(vm, "file", "size", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::Machine::fetch_vm(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto path = vm -> get_string(1);
                        vm -> push_number(static_cast<double>(Vital::Tool::File::size(to_godot_string(get_directory()), to_godot_string(path))));
                        return 1;
                    });
                });
            
                API::bind(vm, "file", "delete", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::Machine::fetch_vm(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto path = vm -> get_string(1);
                        vm -> push_bool(Vital::Tool::File::remove(to_godot_string(get_directory()), to_godot_string(path)));
                        return 1;
                    });
                });
            
                API::bind(vm, "file", "read", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::Machine::fetch_vm(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto path = vm -> get_string(1);
                        auto buffer = Vital::Tool::File::read_text(to_godot_string(get_directory()), to_godot_string(path));
                        vm -> push_string(buffer);
                        return 1;
                    });
                });
            
                API::bind(vm, "file", "write", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::Machine::fetch_vm(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 2) || (!vm -> is_string(1)) || (!vm -> is_string(2))) throw Vital::Error::fetch("invalid-arguments");
                        auto path = vm -> get_string(1);
                        auto buffer = vm -> get_string(2);
                        vm -> push_bool(Vital::Tool::File::write_text(to_godot_string(get_directory()), to_godot_string(path), buffer));
                        return 1;
                    });
                });
            
                API::bind(vm, "file", "contents", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::Machine::fetch_vm(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto path = vm -> get_string(1);
                        bool directory_search = vm -> is_bool(2) ? vm -> get_bool(2) : false;
                        vm -> create_table();
                        for (const auto& i : Vital::Tool::File::contents(to_godot_string(get_directory()), to_godot_string(path), directory_search)) {
                            vm -> table_push_string(i);
                        }
                        return 1;
                    });
                });
            }
    };
}