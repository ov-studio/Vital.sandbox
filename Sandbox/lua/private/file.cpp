/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: private: file.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: File Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include "Sandbox/lua/index.h"


//////////////////////
// Lua: FileSystem //
//////////////////////

namespace Vital::Lua {
    bind("file", "resolve", [](vital_vm* vm) int {
        if (vm -> getArgCount() < 1) {
            vm -> setBool(false);
        }
        else {
            std::string path = vm -> getString(-1);
            Vital::FileSystem::resolve(path);
            vm -> setString(path);
        }
        return 1;
    })

    bind("file", "exists", [](vital_vm* vm) int {
        if (vm -> getArgCount() < 1) {
            vm -> setBool(false);
        }
        else {
            std::string path = vm -> getString(-1);
            vm -> setBool(Vital::FileSystem::exists(path));
        }
        return 1;
    })

    bind("file", "size", [](vital_vm* vm) int {
        // TODO:
    })

    bind("file", "read", [](vital_vm* vm) int {
        // TODO:
    })

    bind("file", "write", [](vital_vm* vm) int {
        // TODO:
    })
}