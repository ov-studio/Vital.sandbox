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
    extern Vital::Type::Inspect::System system();
    extern Vital::Type::Inspect::SMBIOS smbios();
    extern Vital::Type::Inspect::CPU cpu();
    extern std::vector<Vital::Type::Inspect::GPU> gpu();
    extern std::vector<Vital::Type::Inspect::Memory> memory();
    extern std::vector<Vital::Type::Inspect::Network> network();
    extern std::vector<Vital::Type::Inspect::Disk> disk();
}