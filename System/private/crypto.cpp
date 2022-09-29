/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: private: crypto.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Crypto Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/crypto.h>
#include <openssl/sha.h>


////////////////////////
// Namespace: Crypto //
////////////////////////

namespace Vital::Crypto {
    std::string HexToBin(unsigned char* hash, size_t length) {
        std::stringstream ss;
        for (int i = 0; i < length; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        return ss.str();
    }

    std::string SHA256(std::string& buffer) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        ::SHA256(reinterpret_cast<unsigned char*>(const_cast<char*>(buffer.c_str())), buffer.size(), hash);
        return HexToBin(hash, SHA256_DIGEST_LENGTH);
    }
}
