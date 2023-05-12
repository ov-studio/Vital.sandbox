/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: package.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Package System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/vital.h>


/////////////////////////////
// Vital: System: Package //
/////////////////////////////

namespace Vital::System::Package {
    extern bool create(std::string& path, std::vector<Vital::Type::Package::Module> modules, bool isDebugMode = false);
}


/////////////////////////////////////
// Vital: System: Package: Module //
/////////////////////////////////////

namespace Vital::System::Package::Module {
    extern std::string read(std::vector<std::string> buffer);
    extern std::string write(const std::string& buffer, const char delimiter);
}