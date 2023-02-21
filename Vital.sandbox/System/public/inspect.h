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
    extern Vital::System::Inspect::Type::System system();
    extern Vital::System::Inspect::Type::SMBIOS smbios();
    extern Vital::System::Inspect::Type::CPU cpu();
    extern std::vector<Vital::System::Inspect::Type::GPU> gpu();
    extern std::vector<Vital::System::Inspect::Type::Memory> memory();
    extern std::vector<Vital::System::Inspect::Type::Network> network();
    extern std::vector<Vital::System::Inspect::Type::Disk> disk();
}