/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: event.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Event Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/index.h>


/////////////////////////
// Vital: Type: Event //
/////////////////////////

namespace Vital::Type::Event {
    typedef std::map<std::string, std::string> Arguments;
    typedef std::function<void(Arguments)> Handler;
}