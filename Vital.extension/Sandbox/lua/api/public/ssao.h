/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: ssao.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SSAO APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/public/index.h>


////////////////////////////////
// Vital: Sandbox: API: SSAO //
////////////////////////////////

namespace Vital::Sandbox::API {
    class SSAO : public Vital::Tool::Module {
        public:
            static void bind(void* instance);
    };
}