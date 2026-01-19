/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: public: file.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: File APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/lua/public/vm.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Sandbox::Lua::API {
    class File : public Vital::Tool::Module {
        public:
            static void bind(void* instance);
    };
}