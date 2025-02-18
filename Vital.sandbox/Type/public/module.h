/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: module.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Module Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <pch.h>


////////////
// Vital //
////////////

namespace Vital::Type {
    class Module {
        public:
            virtual ~Module() = default;
            static void boot() {};
    };
}

