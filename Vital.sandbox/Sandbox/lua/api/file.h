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
#include <Vital.sandbox/Sandbox/lua/public/vm.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Sandbox::Lua::API {
    class File : public Vital::Tool::Module {
        public:
            inline static void bind(void* instance) {
                auto vm = Vital::Sandbox::Lua::create::toVM(instance);

                API::bind(vm, "file", "exists", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> getArgCount() < 1) || (!vm -> is_string(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto path = vm -> getString(1);
                        vm -> setBool(Vital::Tool::File::exists(to_godot_string(get_directory()), to_godot_string(path)));
                        return 1;
                    });
                });
            
                API::bind(vm, "file", "size", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> getArgCount() < 1) || (!vm -> is_string(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto path = vm -> getString(1);
                        vm -> setNumber(static_cast<double>(Vital::Tool::File::size(to_godot_string(get_directory()), to_godot_string(path))));
                        return 1;
                    });
                });
            
                API::bind(vm, "file", "delete", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> getArgCount() < 1) || (!vm -> is_string(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto path = vm -> getString(1);
                        vm -> setBool(Vital::Tool::File::remove(to_godot_string(get_directory()), to_godot_string(path)));
                        return 1;
                    });
                });
            
                API::bind(vm, "file", "read", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> getArgCount() < 1) || (!vm -> is_string(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto path = vm -> getString(1);
                        auto buffer = Vital::Tool::File::read_text(to_godot_string(get_directory()), to_godot_string(path));
                        vm -> setString(buffer);
                        return 1;
                    });
                });
            
                API::bind(vm, "file", "write", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> getArgCount() < 2) || (!vm -> is_string(1)) || (!vm -> is_string(2))) throw Vital::Error::fetch("invalid-arguments");
                        auto path = vm -> getString(1);
                        auto buffer = vm -> getString(2);
                        vm -> setBool(Vital::Tool::File::write_text(to_godot_string(get_directory()), to_godot_string(path), buffer));
                        return 1;
                    });
                });
            
                API::bind(vm, "file", "contents", [](auto* ref) -> int {
                    auto vm = Vital::Sandbox::Lua::create::fetchVM(ref);
                    return vm -> execute([&]() -> int {
                        if ((vm -> getArgCount() < 1) || (!vm -> is_string(1))) throw Vital::Error::fetch("invalid-arguments");
                        auto path = vm -> getString(1);
                        bool directory_search = vm -> is_bool(2) ? vm -> getBool(2) : false;
                        vm -> createTable();
                        for (const auto& i : Vital::Tool::File::contents(to_godot_string(get_directory()), to_godot_string(path), directory_search)) {
                            vm -> pushString(i);
                        }
                        return 1;
                    });
                });
            }
    };
}