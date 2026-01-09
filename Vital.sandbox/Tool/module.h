/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: public: module.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Module Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <pch.h>


////////////
// Vital //
////////////

namespace Vital::Tool {
    class Module {
        public:
            virtual ~Module() = default;
            static void bind(void* entity) {};
            static void inject(void* entity) {};
    };
}

