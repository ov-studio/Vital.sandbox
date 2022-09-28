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
#include <Vendor/openssl/sha.h>


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

    std::string sha256(std::string str)
    {
        const char* base = str.c_str();
        unsigned char hash[SHA256_DIGEST_LENGTH];
        auto __base = reinterpret_cast<unsigned char*>(const_cast<char*>(base));
        auto buffer = SHA256(__base, str.size(), hash);
        std::cout << buffer;
        return "";
        /*
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, str.c_str(), str.size());
        SHA256_Final(hash, &sha256);
        stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        {
            ss << hex << setw(2) << setfill('0') << (int)hash[i];
        }
        return ss.str();
        */
    }

    /*
    std::string md5(std::string& str) {
        unsigned char hash[MD5_DIGEST_LENGTH];
        MD5_CTX md5;
        md5.
        MD5_Init(&md5);
        MD5_Update(&md5, str.c_str(), str.size());
        MD5_Final(hash, &md5);
        std::stringstream ss;
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        return ss.str();
    }
    */
}