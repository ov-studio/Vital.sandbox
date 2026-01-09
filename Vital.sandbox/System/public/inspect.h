/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: inspect.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Inspect System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/vital.h>


///////////////////////////
// Vital: Type: Inspect //
///////////////////////////

namespace Vital::System::Inspect {
    extern std::string system();
    extern std::string cpu();
    extern std::string disk();
    extern std::string fingerprint();
}