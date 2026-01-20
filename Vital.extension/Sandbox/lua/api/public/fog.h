/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: fog.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Fog APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/public/index.h>


///////////////////////////////
// Vital: Sandbox: API: Fog //
///////////////////////////////

namespace Vital::Sandbox::API {
    class Fog : public Vital::Tool::Module {
        public:
            static void bind(void* instance);
    };
}