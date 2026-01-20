/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: ssil.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SSIL APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/public/api.h>


////////////////////////////////////////////
// Vital: Sandbox: API: SSIL //
////////////////////////////////////////////

namespace Vital::Sandbox::API {
    class SSIL : public Vital::Tool::Module {
        public:
            static void bind(void* instance);
    };
}