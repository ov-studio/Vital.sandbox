/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: module: c++: file.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: File Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/filesystem.h>
#include <Sandbox/lua/public/api.h>


///////////////////////
// Lua: File Binder //
///////////////////////

namespace Vital::Lua::API {
    void vSandbox_File() {
        bind("file", "resolve", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 1) || (!vm -> isString(-1))) {
                vm -> setBool(false);
            }
            else {
                std::string path = vm -> getString(-1);
                Vital::FileSystem::resolve(path);
                vm -> setString(path);
            }
            return 1;
        });

        bind("file", "exists", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 1) || (!vm -> isString(-1))) {
                vm -> setBool(false);
            }
            else {
                std::string path = vm -> getString(-1);
                vm -> setBool(Vital::FileSystem::exists(path));
            }
            return 1;
        });

        bind("file", "size", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 1) || (!vm -> isString(-1))) {
                vm -> setBool(false);
            }
            else {
                std::string path = vm -> getString(-1);
                vm -> setDouble(static_cast<double>(Vital::FileSystem::size(path)));
            }
            return 1;
        });

        bind("file", "delete", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 1) || (!vm -> isString(-1))) {
                vm -> setBool(false);
            }
            else {
                std::string path = vm -> getString(-1);
                vm -> setBool(Vital::FileSystem::remove(path));
            }
            return 1;
        });

        bind("file", "read", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 1) || (!vm -> isString(-1))) {
                vm -> setBool(false);
            }
            else {
                std::string path = vm -> getString(-1);
                if (!Vital::FileSystem::exists(path)) {
                    vm -> setBool(false);
                }
                else {
                    vm -> setString(Vital::FileSystem::read(path));
                }
            }
            return 1;
        });

        bind("file", "write", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 2) || (!vm -> isString(-1)) || (!vm -> isString(-2))) {
                vm -> setBool(false);
            }
            else {
                std::string path = vm -> getString(-1);
                std::string buffer = vm -> getString(-2);
                vm -> setBool(Vital::FileSystem::write(path, reinterpret_cast<char*>(&buffer)));
            }
            return 1;
        });

        bind("file", "fetchContents", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            if ((vm -> getArgCount() < 1) || (!vm -> isString(-1))) {
                vm -> setBool(false);
            }
            else {
                std::string path = vm -> getString(-1);
                vm -> createTable();
                for (auto i : Vital::FileSystem::fetchContents(path)) {
                    vm -> setString(i);
                    vm -> pop(1);
                }
            }
            return 1;
        });
    }
}
