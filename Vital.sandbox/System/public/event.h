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
#include <Vital.sandbox/System/public/vital.h>


/////////////////////////
// Vital: Type: Event //
/////////////////////////

namespace Vital::System::Event {
    bool isEvent(const std::string& identifier);
    extern Vital::Type::Event::Handle bind(const std::string& identifier, Vital::Type::Event::Handler exec);
    extern bool emit(const std::string& identifier, Vital::Type::Stack arguments = {});
}