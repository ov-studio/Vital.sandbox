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
#define AES_BLOCK_SIZE 16
#define AES_128_KEY_SIZE 16
#define AES_256_KEY_SIZE 32


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
    
    const EVP_CIPHER* createCipher(std::string& mode) {
        if (mode == "AES256") return EVP_aes_256_cbc();
        else throw 0;
    }

    std::pair<std::string, std::string> encrypt(std::string& mode, std::string& buffer, std::string& key) {
        EVP_CIPHER* cipherType;
        try { cipherType = const_cast<EVP_CIPHER*>(createCipher(mode)); }
        catch(int error) { throw 0; }
        int __cipherSize, cipherSize;
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        int blockSize = EVP_CIPHER_block_size(cipherType);
        unsigned char* cipher = new unsigned char[(buffer.size() + blockSize)];
        unsigned char* iv = new unsigned char[blockSize];
        RAND_bytes(iv, blockSize);
        EVP_CipherInit(ctx, cipherType, NULL, NULL, 1);
        if (EVP_CIPHER_CTX_key_length(ctx) != key.size()) throw 0;
        EVP_CipherInit(ctx, cipherType, reinterpret_cast<unsigned char*>(const_cast<char*>(key.c_str())), iv, 1);
        EVP_CipherUpdate(ctx, cipher, &__cipherSize, reinterpret_cast<unsigned char*>(const_cast<char*>(buffer.c_str())), buffer.size());
        cipherSize = __cipherSize;
        EVP_CipherFinal(ctx, cipher + __cipherSize, &__cipherSize);
        cipherSize += __cipherSize;
        cipher[cipherSize] = 0;
        std::pair<std::string, std::string> result = {reinterpret_cast<const char*>(cipher), reinterpret_cast<const char*>(iv)};
        delete[] iv, cipher;
        std::cout << "AES (Encrypt): " << cipher << "\n";
        //decrypt(result.first, key, result.second);
        return result;
    }

    std::string decrypt(std::string& mode, std::string& buffer, std::string& key, std::string& iv) {
        EVP_CIPHER* cipherType;
        try { cipherType = const_cast<EVP_CIPHER*>(createCipher(mode)); }
        catch(int error) { throw 0; }
        int __cipherSize, cipherSize;
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        int blockSize = EVP_CIPHER_block_size(cipherType);
        unsigned char* cipher = new unsigned char[(buffer.size() + blockSize)];
        EVP_CipherInit(ctx, cipherType, NULL, NULL, 0);
        if ((EVP_CIPHER_CTX_key_length(ctx) != key.size()) || (EVP_CIPHER_CTX_iv_length(ctx) != iv.size())) throw 0;
        EVP_CipherInit(ctx, cipherType, reinterpret_cast<unsigned char*>(const_cast<char*>(key.c_str())), reinterpret_cast<unsigned char*>(const_cast<char*>(iv.c_str())), 0);
        EVP_CipherUpdate(ctx, cipher, &__cipherSize, reinterpret_cast<unsigned char*>(const_cast<char*>(buffer.c_str())), buffer.size());
        cipherSize = __cipherSize;
        EVP_CipherFinal(ctx, cipher + __cipherSize, &__cipherSize);
        cipherSize += __cipherSize;
        cipher[cipherSize] = 0;
        std::cout << "\nAES (Decrypt): " << cipher << " Length: " << cipherSize;
        return reinterpret_cast<const char*>(cipher);
    }
}
