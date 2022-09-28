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
#include <Vendor/openssl/sha.h>


////////////////////////
// Namespace: Crypto //
////////////////////////

namespace Vital::Crypto {
    std::string SHA256(std::string& buffer) {
        const char* base = buffer.c_str();
        unsigned char hash[SHA256_DIGEST_LENGTH];
        auto __base = reinterpret_cast<unsigned char*>(const_cast<char*>(base));
        return SHA256(__base, buffer.size(), hash);
    }
}
