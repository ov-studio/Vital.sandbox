/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: adjustment.h
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


//////////////////////////////////////
// Vital: Sandbox: API: Adjustment //
//////////////////////////////////////

namespace Vital::Sandbox::API {
    class Adjustment : public Vital::Tool::Module {
        public:
            static void bind(void* instance);
    };
}