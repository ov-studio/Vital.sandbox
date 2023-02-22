/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: event.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Event System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/vital.h>


/////////////////////////
// Vital: Type: Event //
/////////////////////////

namespace Vital::System::Event {
    // Instantiators //
    extern bool create(const std::string& name);
    extern bool destroy(const std::string& name);

    // APIs //
    extern bool bind(const std::string& name, Vital::Type::Event::Handler exec);
    extern bool unbind(const std::string& name, Vital::Type::Event::Handler exec);
    extern bool emit(const std::string& name, Vital::Type::Event::Arguments arguments);
}