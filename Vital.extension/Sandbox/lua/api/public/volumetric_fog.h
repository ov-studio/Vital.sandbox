/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: lua: api: volumetric_fog.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Volumetric Fog APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/public/api.h>


/////////////////////////////////////////////////////
// Vital: Godot: Sandbox: Lua: API: VolumetricFog //
/////////////////////////////////////////////////////

namespace Vital::Godot::Sandbox::Lua::API {
    class VolumetricFog : public Vital::Tool::Module {
        public:
            static void bind(void* instance);
    };
}