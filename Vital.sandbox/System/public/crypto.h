/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: public: crypto.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Crypto System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/System/public/vital.h>


////////////////////////////
// Vital: Tool: Crypto //
////////////////////////////

namespace Vital::System::Crypto {
    extern std::string hash(const std::string& mode, const std::string& buffer);
    extern std::string encode(const std::string& buffer);
    extern std::string decode(const std::string& buffer);
    extern std::pair<std::string, std::string> encrypt(const std::string& mode, const std::string& buffer, const std::string& key);
    extern std::string decrypt(const std::string& mode, const std::string& buffer, const std::string& key, const std::string& iv);
}