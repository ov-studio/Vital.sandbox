/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: lua: api: ssr.h
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
// Vital: Godot: Sandbox: Lua: API: SSR //
///////////////////////////////////////////

namespace Vital::Godot::Sandbox::Lua::API {
    class SSR : public Vital::Tool::Module {
        public:
            static void bind(void* instance);
    };
}