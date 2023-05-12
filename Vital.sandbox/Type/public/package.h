/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: package.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Package Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/index.h>


///////////////////////////
// Vital: Type: Package //
///////////////////////////

namespace Vital::Type::Package {
    typedef struct Module {
        std::string identifier;
        std::string path;
        char delimiter;
    } Module;
}