/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: lua: api: adjustment.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Adjustment APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/public/api.h>


//////////////////////////////////////////////////
// Vital: Godot: Sandbox: Machine: API: Adjustment //
//////////////////////////////////////////////////

namespace Vital::Godot::Sandbox::Lua::API {
    class Adjustment : public Vital::Tool::Module {
        public:
            static void bind(void* instance);
    };
}