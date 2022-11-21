/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: module: c++: file.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: File Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/file.h>
#include <Sandbox/lua/public/api.h>


///////////////////////
// Lua: File Binder //
///////////////////////

namespace Vital::Sandbox::Lua::API {
    static bool isBound = false;
    void vSandbox_File() {
        if (isBound) return; isBound = true;

        bind("file", "resolve", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw ErrorCode["invalid-arguments"];
                std::string path = vm -> getString(1);
                Vital::System::File::resolve(path);
                vm -> setString(path);
                return 1;
            });
        });

        bind("file", "exists", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw ErrorCode["invalid-arguments"];
                std::string path = vm -> getString(1);
                vm -> setBool(Vital::System::File::exists(path));
                return 1;
            });
        });

        bind("file", "size", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw ErrorCode["invalid-arguments"];
                std::string path = vm -> getString(1);
                vm -> setNumber(static_cast<double>(Vital::System::File::size(path)));
                return 1;
            });
        });

        bind("file", "delete", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw ErrorCode["invalid-arguments"];
                std::string path = vm -> getString(1);
                vm -> setBool(Vital::System::File::remove(path));
                return 1;
            });
        });

        bind("file", "read", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw ErrorCode["invalid-arguments"];
                std::string path = vm -> getString(1);
                auto buffer = Vital::System::File::read(path);
                vm -> setString(buffer);
                return 1;
            });
        });

        bind("file", "write", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isString(1)) || (!vm -> isString(2))) throw ErrorCode["invalid-arguments"];
                std::string path = vm -> getString(1);
                std::string buffer = vm -> getString(2);
                vm -> setBool(Vital::System::File::write(path, reinterpret_cast<char*>(&buffer)));
                return 1;
            });
        });

        bind("file", "contents", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw ErrorCode["invalid-arguments"];
                std::string path = vm -> getString(1);
                bool fetchDirs = vm -> isBool(2) ? vm -> getBool(2) : false;
                vm -> createTable();
                for (auto& i : Vital::System::File::contents(path, fetchDirs)) {
                    vm -> pushString(i);
                }
                return 1;
            });
        });
    }
}
