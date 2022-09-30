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

    const EVP_CIPHER* CipherMode(std::string& mode) {
        if (mode == "AES128") return EVP_aes_128_cbc();
        else if (mode == "AES192") return EVP_aes_192_cbc();
        else if (mode == "AES256") return EVP_aes_256_cbc();
        else throw 0;
    }

    std::string CipherIV(std::string& mode) {
        try {
            EVP_CIPHER* cipherType = const_cast<EVP_CIPHER*>(CipherMode(mode));
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            int ivSize = EVP_CIPHER_block_size(cipherType);
            unsigned char* iv = new unsigned char[ivSize];
            RAND_bytes(iv, ivSize);
            iv[ivSize] = 0;
            std::string result = reinterpret_cast<const char*>(iv);
            EVP_CIPHER_CTX_free(ctx);
            delete[] iv;
            return result;
        }
        catch(int error) { throw error; }
    }

    std::string CipherHandle(std::string& mode, bool isEncrypt, std::string& buffer, std::string& key, std::string& iv) {
        try {
            EVP_CIPHER* cipherType = const_cast<EVP_CIPHER*>(CipherMode(mode));
            int __cipherSize, cipherSize;
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            int cipherMode = static_cast<int>(isEncrypt);
            int blockSize = EVP_CIPHER_block_size(cipherType);
            unsigned char* cipher = new unsigned char[(buffer.size() + blockSize)];
            EVP_CipherInit(ctx, cipherType, NULL, NULL, cipherMode);
            if ((EVP_CIPHER_CTX_key_length(ctx) != key.size()) || (EVP_CIPHER_CTX_iv_length(ctx) != iv.size())) throw 0;
            EVP_CipherInit(ctx, cipherType, reinterpret_cast<unsigned char*>(const_cast<char*>(key.c_str())), reinterpret_cast<unsigned char*>(const_cast<char*>(iv.c_str())), cipherMode);
            EVP_CipherUpdate(ctx, cipher, &__cipherSize, reinterpret_cast<unsigned char*>(const_cast<char*>(buffer.c_str())), static_cast<int>(buffer.size()));
            cipherSize = __cipherSize;
            EVP_CipherFinal(ctx, cipher + __cipherSize, &__cipherSize);
            cipherSize += __cipherSize;
            cipher[cipherSize] = 0;
            EVP_CIPHER_CTX_free(ctx);
            return reinterpret_cast<const char*>(cipher);
        }
        catch(int error) { throw error; }
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

    std::pair<std::string, std::string> encrypt(std::string mode, std::string& buffer, std::string& key) {
        try {
            std::string iv = CipherIV(mode);
            std::string result = CipherHandle(mode, true, buffer, key, iv);
            return {result, iv};
        }
        catch(int error) { throw error; }
    }

    std::string decrypt(std::string mode, std::string& buffer, std::string& key, std::string& iv) {
        try { return CipherHandle(mode, false, buffer, key, iv); }
        catch(int error) { throw error; }
    }
}
