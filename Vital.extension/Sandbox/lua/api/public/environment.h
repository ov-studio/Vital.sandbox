/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: lua: api: public: environment.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Environment APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/public/api.h>


///////////////////////////////////////////////////
// Vital: Godot: Sandbox: Lua: API: Environment //
///////////////////////////////////////////////////

namespace Vital::Godot::Sandbox::Lua::API {
    class Environment : public Vital::Type::Module {
        public:
            static void bind(void* instance);
    };
}