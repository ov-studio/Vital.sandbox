/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: module: public: engine.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Engine APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Sandbox/lua/public/api.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Sandbox::Lua::API {
    class Engine : public Vital::Type::Module {
        private:
            static bool bound;
        public:
            static void boot();
    };
}