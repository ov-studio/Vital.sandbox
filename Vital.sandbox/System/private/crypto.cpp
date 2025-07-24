/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: private: crypto.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
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
#include <openssl/bio.h>
#include <openssl/buffer.h>


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
        else throw std::runtime_error(fmt::format(ErrorCode["hash-mode-nonexistent"], mode));
    }

    const EVP_CIPHER* CipherMode(const std::string& mode) {
        if (mode == "AES128") return EVP_aes_128_cbc();
        else if (mode == "AES192") return EVP_aes_192_cbc();
        else if (mode == "AES256") return EVP_aes_256_cbc();
        else throw std::runtime_error(fmt::format(ErrorCode["cipher-mode-nonexistent"], mode));
    }

    std::string CipherIV(const std::string& mode) {
        try {
            EVP_CIPHER* algorithm = const_cast<EVP_CIPHER*>(CipherMode(mode));
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            int ivSize = EVP_CIPHER_block_size(algorithm) + 1;
            unsigned char* iv = new unsigned char[ivSize];
            RAND_bytes(iv, ivSize);
            std::string result = std::string(reinterpret_cast<char*>(iv), ivSize);
            EVP_CIPHER_CTX_free(ctx);
            delete[] iv;
            return result;
        }
        catch(...) { std::rethrow_exception(std::current_exception()); }
    }

    std::string CipherHandle(const std::string& mode, bool isEncrypt, const std::string& buffer, const std::string& key, const std::string& iv) {
        try {
            EVP_CIPHER* algorithm = const_cast<EVP_CIPHER*>(CipherMode(mode));
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            auto EVP_Init = isEncrypt ? EVP_EncryptInit : EVP_DecryptInit;
            auto EVP_Update = isEncrypt ? EVP_EncryptUpdate : EVP_DecryptUpdate;
            auto EVP_Final = isEncrypt ? EVP_EncryptFinal : EVP_DecryptFinal;
            int blockSize = EVP_CIPHER_block_size(algorithm);
            int inputSize = static_cast<unsigned int>(buffer.size()) + 1, outputSize = 0, currentSize = 0;
            int bufferSize = inputSize + blockSize - 1;
            unsigned char* output = new unsigned char[bufferSize];
            EVP_Init(ctx, algorithm, NULL, NULL);
            if (EVP_CIPHER_CTX_key_length(ctx) != key.size()) throw std::runtime_error(ErrorCode["cipher-invalid-key"]);
            if (EVP_CIPHER_CTX_iv_length(ctx) != iv.size()) throw std::runtime_error(ErrorCode["cipher-invalid-iv"]);
            EVP_Init(ctx, algorithm, reinterpret_cast<unsigned char*>(const_cast<char*>(key.data())), reinterpret_cast<unsigned char*>(const_cast<char*>(iv.data())));
            EVP_Update(ctx, output, &currentSize, reinterpret_cast<unsigned char*>(const_cast<char*>(buffer.data())), inputSize);
            outputSize += currentSize;
            EVP_Final(ctx, output + currentSize, &currentSize);
            outputSize += currentSize;
            EVP_CIPHER_CTX_free(ctx);
            std::string result = std::string(reinterpret_cast<char*>(output), outputSize);
            delete[] output;
            return result;
        }
        catch(...) { std::rethrow_exception(std::current_exception()); }
    }

	std::string hash(const std::string& mode, const std::string& buffer) {
        try {
            auto algorithm = HashMode(mode);
            int outputSize = algorithm.second;
            std::vector<unsigned char> output(outputSize);
            algorithm.first(reinterpret_cast<const unsigned char*>(buffer.data()), buffer.size(), output.data());
            std::stringstream ss;
            for (const auto& byte : output) {
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
            }
            return ss.str();
        }
        catch(...) { std::rethrow_exception(std::current_exception()); }
    }

    std::string encode(const std::string& buffer) {
        BIO* bio = BIO_new(BIO_f_base64());
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO* bmem = BIO_new(BIO_s_mem());
        bio = BIO_push(bio, bmem);
        BIO_write(bio, buffer.c_str(), buffer.size());
        BIO_flush(bio);
        BUF_MEM* bptr;
        BIO_get_mem_ptr(bio, &bptr);
        std::string result(bptr->data, bptr->length);
        BIO_free_all(bio);
        return result;
    }
    
    std::string decode(const std::string& buffer) {
        BIO* bio = BIO_new(BIO_f_base64());
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO* bmem = BIO_new_mem_buf(buffer.c_str(), buffer.size());
        bio = BIO_push(bio, bmem);
        std::string result(buffer.size(), '\0');
        int decodedLength = BIO_read(bio, &result[0], buffer.size());
        BIO_free_all(bio);
        result.resize(decodedLength);
        return result;
    }

    std::pair<std::string, std::string> encrypt(const std::string& mode, const std::string& buffer, const std::string& key) {
        try {
            std::string iv = CipherIV(mode);
            std::string result = CipherHandle(mode, true, buffer, key, iv);
            return {encode(result), encode(iv)};
        }
        catch(...) { std::rethrow_exception(std::current_exception()); }
    }

    std::string decrypt(const std::string& mode, const std::string& buffer, const std::string& key, const std::string& iv) {
        try {
            return CipherHandle(mode, false, decode(buffer), key, decode(iv));
        }
        catch(...) { std::rethrow_exception(std::current_exception()); }
    }
}
