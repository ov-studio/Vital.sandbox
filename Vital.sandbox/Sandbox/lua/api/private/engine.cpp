/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: private: engine.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Engine APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/lua/api/public/engine.h>


///////////////
// Lua: API //
///////////////

void Vital::Sandbox::Lua::API::Engine::bind(void* instance) {

}

void Vital::Sandbox::Lua::API::Engine::inject(void* instance) {
    auto vm = Vital::Sandbox::Lua::toVM(instance);
    #if defined(Vital_SDK_Client)
    vm -> getGlobal("engine");
    vm -> getTableField("print", -1);
    vm -> setGlobal("print");
    vm -> pop(1);
    #endif
}
