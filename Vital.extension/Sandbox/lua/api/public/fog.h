/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: lua: api: fog.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Fog APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/public/api.h>


///////////////////////////////////////////
// Vital: Godot: Sandbox: Machine: API: Fog //
///////////////////////////////////////////

namespace Vital::Godot::Sandbox::Lua::API {
    class Fog : public Vital::Tool::Module {
        public:
            static void bind(void* instance);
    };
}