/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: crypto.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Crypto Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <pch.h>
#include <System/public/vital.h>


////////////////////////
// Namespace: Crypto //
////////////////////////

namespace Vital::Crypto {
    extern std::string SHA1(std::string& buffer);
    extern std::string SHA224(std::string& buffer);
    extern std::string SHA256(std::string& buffer);
    extern std::string SHA384(std::string& buffer);
    extern std::string SHA512(std::string& buffer);
    extern std::pair<std::string, std::string> encrypt(std::string mode, std::string& buffer, std::string& key);
    extern std::string decrypt(std::string mode, std::string& buffer, std::string& key, unsigned char* iv);
}