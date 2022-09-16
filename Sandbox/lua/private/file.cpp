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
    bind("file", "resolve", [](vital_vm* vm) {
        // TODO:
    })

    bind("file", "exists", [](vital_vm* vm) {
        // TODO:
    })

    bind("file", "size", [](vital_vm* vm) {
        // TODO:
    })

    bind("file", "read", [](vital_vm* vm) {
        // TODO:
    })

    bind("file", "write", [](vital_vm* vm) {
        // TODO:
    })
}