/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: private: filesystem.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: File System Handler
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
    bind("file", "getPath", [](vital_vm* vm) {
        // TODO:
    })

    bind("file", "getSize", [](vital_vm* vm) {
        // TODO:
    })

    bind("file", "exists", [](vital_vm* vm) {
        // TODO:
    })

    bind("file", "read", [](vital_vm* vm) {
        // TODO:
    })

    bind("file", "write", [](vital_vm* vm) {
        // TODO:
    })
}