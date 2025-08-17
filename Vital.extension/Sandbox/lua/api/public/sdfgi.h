/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: lua: api: public: sdfgi.h
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


/////////////////////////////////////////////
// Vital: Godot: Sandbox: Lua: API: SDFGI //
/////////////////////////////////////////////

namespace Vital::Godot::Sandbox::Lua::API {
    class SDFGI : public Vital::Type::Module {
        public:
            static void bind(void* instance);
    };
}