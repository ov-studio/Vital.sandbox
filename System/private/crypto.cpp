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
    std::string HexToBin(unsigned char* buffer, size_t length) {
        std::stringstream result;
        for (int i = 0; i < length; i++) {
            result << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[i]);
        }
        return result.str();
    }

    std::pair<std::function<unsigned char* (const unsigned char*, size_t, unsigned char*)>, int> HashMode(std::string& mode) {
        if (mode == "SHA1") return {::SHA1, SHA_DIGEST_LENGTH};
        else if (mode == "SHA224") return {::SHA224, SHA224_DIGEST_LENGTH};
        else if (mode == "SHA256") return {::SHA256, SHA256_DIGEST_LENGTH};
        else if (mode == "SHA384") return {::SHA384, SHA384_DIGEST_LENGTH};
        else if (mode == "SHA512") return {::SHA512, SHA512_DIGEST_LENGTH};
        else throw 0;
    }

    const EVP_CIPHER* CipherMode(std::string& mode) {
        if (mode == "AES128") return EVP_aes_128_cbc();
        else if (mode == "AES192") return EVP_aes_192_cbc();
        else if (mode == "AES256") return EVP_aes_256_cbc();
        else throw 0;
    }

    std::string CipherIV(std::string& mode) {
        try {
            EVP_CIPHER* algorithm = const_cast<EVP_CIPHER*>(CipherMode(mode));
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            int ivSize = EVP_CIPHER_block_size(algorithm);
            unsigned char* iv = new unsigned char[(ivSize + 1)];
            RAND_bytes(iv, ivSize);
            std::string result = std::string(reinterpret_cast<char const*>(iv), ivSize);
            EVP_CIPHER_CTX_free(ctx);
            delete[] iv;
            return result;
        }
        catch(int error) { throw error; }
    }

    std::string CipherHandle(std::string& mode, bool isEncrypt, std::string& buffer, std::string& key, std::string& iv) {
        try {
            // Init CTX //
            EVP_CIPHER* algorithm = const_cast<EVP_CIPHER*>(CipherMode(mode));
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            auto EVP_Init = isEncrypt ? EVP_EncryptInit : EVP_DecryptInit;
            auto EVP_Update = isEncrypt ? EVP_EncryptUpdate : EVP_DecryptUpdate;
            auto EVP_Final = isEncrypt ? EVP_EncryptFinal : EVP_DecryptFinal;
            int blockSize = EVP_CIPHER_block_size(algorithm);
            int inputSize = buffer.size() + 1, outputSize = 0, currentSize = 0;
            unsigned char* output = new unsigned char[(inputSize + blockSize - 1)];
            // Handle CTX //
            EVP_Init(ctx, algorithm, NULL, NULL);
            if ((EVP_CIPHER_CTX_key_length(ctx) != key.size()) || (EVP_CIPHER_CTX_iv_length(ctx) != iv.size())) throw 0;
            EVP_Init(ctx, algorithm, reinterpret_cast<unsigned char*>(key.data()), reinterpret_cast<unsigned char*>(iv.data()));
            EVP_Update(ctx, output, &currentSize, reinterpret_cast<unsigned char*>(buffer.data()), inputSize);
            outputSize += currentSize;
            EVP_Final(ctx, output + currentSize, &currentSize);
            outputSize += currentSize;
            // Cleanup CTX //
            std::string result = std::string(reinterpret_cast<char const*>(output), outputSize);
            EVP_CIPHER_CTX_free(ctx);
            std::cout << "ORIG LENGTH: " << inputSize << " - NEW LENGTH: " << outputSize << "\n";
            std::cout << "String Length: " << result.size() << "\n" << result << "\n\n-------------\n\n";
            return result;
        }
        catch(int error) { throw error; }
    }

    std::string hash(std::string mode, std::string& buffer) {
        try {
            auto algorithm = HashMode(mode);
            int outputSize = algorithm.second;
            unsigned char* output = new unsigned char[outputSize];
            algorithm.first(reinterpret_cast<unsigned char*>(buffer.data()), buffer.size(), output);
            std::string result = HexToBin(output, outputSize);
            delete[] output;
            return result;
        }
        catch(int error) { throw error; }
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
