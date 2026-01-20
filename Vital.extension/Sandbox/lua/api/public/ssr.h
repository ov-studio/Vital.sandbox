/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: ssr.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SSR APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/public/api.h>


///////////////////////////////////////////
// Vital: Sandbox: API: SSR //
///////////////////////////////////////////

namespace Vital::Sandbox::API {
    class SSR : public Vital::Tool::Module {
        public:
            static void bind(void* instance);
    };
}