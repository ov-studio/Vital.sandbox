/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: private: crypto.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Crypto System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/crypto.h>
#include <openssl/rand.h>
#include <openssl/sha.h>


////////////////////////////
// Vital: System: Crypto //
////////////////////////////

namespace Vital::System::Crypto {
    std::pair<std::function<unsigned char* (const unsigned char*, size_t, unsigned char*)>, int> HashMode(const std::string& mode) {
        if (mode == "SHA1") return {::SHA1, SHA_DIGEST_LENGTH};
        else if (mode == "SHA224") return {::SHA224, SHA224_DIGEST_LENGTH};
        else if (mode == "SHA256") return {::SHA256, SHA256_DIGEST_LENGTH};
        else if (mode == "SHA384") return {::SHA384, SHA384_DIGEST_LENGTH};
        else if (mode == "SHA512") return {::SHA512, SHA512_DIGEST_LENGTH};
        else throw ErrorCode["hash-mode-nonexistent"];
    }

    const EVP_CIPHER* CipherMode(const std::string& mode) {
        if (mode == "AES128") return EVP_aes_128_cbc();
        else if (mode == "AES192") return EVP_aes_192_cbc();
        else if (mode == "AES256") return EVP_aes_256_cbc();
        else throw ErrorCode["cipher-mode-nonexistent"];
    }

    std::string CipherIV(const std::string& mode) {
        try {
            EVP_CIPHER* algorithm = const_cast<EVP_CIPHER*>(CipherMode(mode));
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            int ivSize = EVP_CIPHER_block_size(algorithm);
            unsigned char* iv = new unsigned char[(ivSize + 1)];
            RAND_bytes(iv, ivSize);
            std::string result = std::string(reinterpret_cast<char*>(iv), ivSize);
            EVP_CIPHER_CTX_free(ctx);
            delete[] iv;
            return result;
        }
        catch(std::string error) { throw error; }
    }

    std::string CipherHandle(const std::string& mode, bool isEncrypt, const std::string& buffer, const std::string& key, const std::string& iv) {
        try {
            EVP_CIPHER* algorithm = const_cast<EVP_CIPHER*>(CipherMode(mode));
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            auto EVP_Init = isEncrypt ? EVP_EncryptInit : EVP_DecryptInit;
            auto EVP_Update = isEncrypt ? EVP_EncryptUpdate : EVP_DecryptUpdate;
            auto EVP_Final = isEncrypt ? EVP_EncryptFinal : EVP_DecryptFinal;
            int blockSize = EVP_CIPHER_block_size(algorithm);
            int inputSize = static_cast<int>(buffer.size()) + 1, outputSize = 0, currentSize = 0;
            unsigned char* output = new unsigned char[(inputSize + blockSize - 1)];
            EVP_Init(ctx, algorithm, NULL, NULL);
            if (EVP_CIPHER_CTX_key_length(ctx) != key.size()) throw ErrorCode["cipher-invalid-key"];
            if (EVP_CIPHER_CTX_iv_length(ctx) != iv.size()) throw ErrorCode["cipher-invalid-iv"];
            EVP_Init(ctx, algorithm, reinterpret_cast<unsigned char*>(const_cast<char*>(key.data())), reinterpret_cast<unsigned char*>(const_cast<char*>(iv.data())));
            EVP_Update(ctx, output, &currentSize, reinterpret_cast<unsigned char*>(const_cast<char*>(buffer.data())), inputSize);
            outputSize += currentSize;
            EVP_Final(ctx, output + currentSize, &currentSize);
            outputSize += currentSize;
            std::string result = std::string(reinterpret_cast<char*>(output), outputSize);
            EVP_CIPHER_CTX_free(ctx);
            return result;
        }
        catch(std::string error) { throw error; }
    }

    std::string hash(const std::string mode, const std::string& buffer) {
        try {
            auto algorithm = HashMode(mode);
            int outputSize = algorithm.second;
            unsigned char* output = new unsigned char[outputSize];
            algorithm.first(reinterpret_cast<unsigned char*>(const_cast<char*>(buffer.data())), buffer.size(), output);
            std::string result = std::string(reinterpret_cast<char*>(output), outputSize);
            result = encode(result);
            delete[] output;
            return result;
        }
        catch(std::string error) { throw error; }
    }

    std::string encode(std::string& buffer) {
        std::stringstream result;
        for (unsigned char i : buffer) {
            result << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(i);
        }
        return result.str();
    }

    std::string decode(const std::string& buffer) {
        try {
            int bufferSize = static_cast<int>(buffer.size());
            std::string result;
            result.reserve(bufferSize/2);
            for (std::string::const_iterator i = buffer.begin(); i < buffer.end(); i += 2) {
                result.push_back(std::stoi(std::string(i, i + 2), nullptr, 16));
            }
            return result;
        }
        catch(...) { throw ErrorCode["decode-failed"]; }
    }

    std::pair<std::string, std::string> encrypt(const std::string mode, const std::string& buffer, const std::string& key) {
        try {
            std::string iv = CipherIV(mode);
            std::string result = CipherHandle(mode, true, buffer, key, iv);
            return {encode(result), encode(iv)};
        }
        catch(std::string error) { throw error; }
    }

    std::string decrypt(const std::string mode, const std::string& buffer, const std::string& key, const std::string& iv) {
        try {
            std::string __buffer = decode(buffer);
            std::string __iv = decode(iv);
            return CipherHandle(mode, false, __buffer, key, __iv);
        }
        catch(std::string error) { throw error; }
    }
}
