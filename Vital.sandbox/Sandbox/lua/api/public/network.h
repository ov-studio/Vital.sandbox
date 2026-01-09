/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: public: network.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network APIs
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
    class Network : public Vital::Tool::Module {
        public:
            static void bind(void* instance);
            static void inject(void* instance);
            static void execute(const std::string& name, const std::string& payload);
    };
}