/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: vital.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Vital Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/vital.h>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>


///////////////////////
// Namespace: Vital //
///////////////////////

namespace Vital {
    unsigned int getSystemTick() {
        return static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count() / 1000000);
    }
    unsigned int getApplicationTick() {
        vTick = vTick ? vTick : getSystemTick();
        return static_cast<unsigned int>(getSystemTick() - vTick);
    }

    std::string HexToBin(unsigned char* hash, int length) {
        std::stringstream ss;
        for (int i = 0; i < length; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        return ss.str();
    }

    std::string sha256(std::string str) {
        const char* base = str.c_str();
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<unsigned char*>(const_cast<char*>(base)), str.size(), hash);
        return HexToBin(hash, SHA256_DIGEST_LENGTH);
    }
}