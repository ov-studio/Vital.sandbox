/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: volumetric_fog.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Volumetric Fog APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/public/index.h>


//////////////////////////////////////////
// Vital: Sandbox: API: Volumetric_Fog //
//////////////////////////////////////////

namespace Vital::Sandbox::API {
    class Volumetric_Fog : public Vital::Tool::Module {
        public:
            static void bind(void* machine);
    };
}