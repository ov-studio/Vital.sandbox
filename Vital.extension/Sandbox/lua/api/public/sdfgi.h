/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: sdfgi.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SDFGI APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/public/api.h>


/////////////////////////////////
// Vital: Sandbox: API: SDFGI //
/////////////////////////////////

namespace Vital::Sandbox::API {
    class SDFGI : public Vital::Tool::Module {
        public:
            static void bind(void* instance);
    };
}