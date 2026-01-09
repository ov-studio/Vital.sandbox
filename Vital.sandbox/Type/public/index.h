/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: index.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Type Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <pch.h>


////////////
// Vital //
////////////

namespace Vital {
    static const std::string Signature = "vsdk_v.0.0.1";
    static const std::string Repository = "https://raw.githubusercontent.com/ov-studio/Vital.sandbox/refs/heads/module/{}";

    namespace Type {}
    namespace System {}
    namespace Sandbox {
        extern std::vector<std::string> fetchModules(const std::string& name);
    }
}