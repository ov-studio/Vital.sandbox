/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: emissive.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Emissive APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/public/api.h>


////////////////////////////////////
// Vital: Sandbox: API: Emissive //
////////////////////////////////////

namespace Vital::Sandbox::API {
    class Emissive : public Vital::Tool::Module {
        public:
            static void bind(void* instance);
    };
}