/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: public: crypto.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Crypto APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/lua/public/api.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Sandbox::Lua::API {
    class Crypto : public Vital::Tool::Module {
        public:
            static void bind(void* instance);
    };
}