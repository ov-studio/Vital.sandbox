/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: lua: api: public: ssao.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SSAO APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/public/api.h>


////////////////////////////////////////////
// Vital: Godot: Sandbox: Lua: API: SSAO //
////////////////////////////////////////////

namespace Vital::Godot::Sandbox::Lua::API {
    class SSAO : public Vital::Type::Module {
        public:
            static void bind(void* instance);
    };
}