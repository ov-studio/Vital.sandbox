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
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/aes.h>


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

    std::string SHA1(std::string& buffer) {
        unsigned char hash[SHA_DIGEST_LENGTH];
        ::SHA1(reinterpret_cast<unsigned char*>(const_cast<char*>(buffer.c_str())), buffer.size(), hash);
        return HexToBin(hash, SHA_DIGEST_LENGTH);
    }

    std::string SHA224(std::string& buffer) {
        unsigned char hash[SHA224_DIGEST_LENGTH];
        ::SHA224(reinterpret_cast<unsigned char*>(const_cast<char*>(buffer.c_str())), buffer.size(), hash);
        return HexToBin(hash, SHA224_DIGEST_LENGTH);
    }

    std::string SHA256(std::string& buffer) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        ::SHA256(reinterpret_cast<unsigned char*>(const_cast<char*>(buffer.c_str())), buffer.size(), hash);
        return HexToBin(hash, SHA256_DIGEST_LENGTH);
    }

    std::string SHA384(std::string& buffer) {
        unsigned char hash[SHA384_DIGEST_LENGTH];
        ::SHA384(reinterpret_cast<unsigned char*>(const_cast<char*>(buffer.c_str())), buffer.size(), hash);
        return HexToBin(hash, SHA384_DIGEST_LENGTH);
    }

    std::string SHA512(std::string& buffer) {
        unsigned char hash[SHA512_DIGEST_LENGTH];
        ::SHA512(reinterpret_cast<unsigned char*>(const_cast<char*>(buffer.c_str())), buffer.size(), hash);
        return HexToBin(hash, SHA512_DIGEST_LENGTH);
    }

    std::vector<std::string, std::string> AES256::encrypt(std::string& buffer, std::string& key) {
        int len;
        int ciphertext_len;
        unsigned char* ciphertext;
        EVP_CIPHER_CTX* ctx;
        unsigned char iv[AES_BLOCK_SIZE];
        RAND_pseudo_bytes(iv, sizeof(iv));
        EVP_EncryptInit(ctx, EVP_aes_256_cbc(), reinterpret_cast<unsigned char*>(const_cast<char*>(key.c_str())), iv);
        EVP_EncryptUpdate(ctx, ciphertext, &len, reinterpret_cast<unsigned char*>(const_cast<char*>(buffer.c_str())), buffer.size());
        ciphertext_len = len;
        EVP_EncryptFinal(ctx, ciphertext + len, &len);
        ciphertext_len += len;
        std::cout << ciphertext;
        //return ciphertext_len;
        return { "", "" };
    }
}
