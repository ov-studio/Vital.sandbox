/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: crypto.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Crypto Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/log.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>


//////////////////////////
// Vital: Tool: Crypto //
//////////////////////////

namespace Vital::Tool::Crypto {
    namespace Internal {
        inline const EVP_MD* hash_mode(std::string_view mode) {
            if (mode == "SHA1") return EVP_sha1();
            if (mode == "SHA224") return EVP_sha224();
            if (mode == "SHA256") return EVP_sha256();
            if (mode == "SHA384") return EVP_sha384();
            if (mode == "SHA512") return EVP_sha512();
            throw Vital::Log::fetch("hash-mode-nonexistent", Vital::Log::Type::Error, mode);
        }

        inline const EVP_CIPHER* cipher_mode(std::string_view mode) {
            if (mode == "AES128") return EVP_aes_128_cbc();
            if (mode == "AES192") return EVP_aes_192_cbc();
            if (mode == "AES256") return EVP_aes_256_cbc();
            throw Vital::Log::fetch("cipher-mode-nonexistent", Vital::Log::Type::Error, mode);
        }

        inline std::string base64_encode(std::string_view in) {
            BIO* b64 = BIO_new(BIO_f_base64());
            BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
            BIO* mem = BIO_new(BIO_s_mem());
            BIO_push(b64, mem);
            BIO_write(b64, in.data(), static_cast<int>(in.size()));
            BIO_flush(b64);
            BUF_MEM* buf{};
            BIO_get_mem_ptr(b64, &buf);
            std::string out(buf->data, buf->length);
            BIO_free_all(b64);
            return out;
        }

        inline std::string base64_decode(std::string_view in) {
            BIO* b64 = BIO_new(BIO_f_base64());
            BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
            BIO* mem = BIO_new_mem_buf(in.data(), static_cast<int>(in.size()));
            BIO_push(b64, mem);
            std::string out(in.size(), '\0');
            int len = BIO_read(b64, out.data(), static_cast<int>(out.size()));
            BIO_free_all(b64);
            out.resize(len > 0 ? len : 0);
            return out;
        }

        inline std::string cipher_run(const EVP_CIPHER* algo, bool encrypt, std::string_view data, std::string_view key, std::string_view iv) {
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            if (!ctx) throw Vital::Log::fetch("cipher-context-failed", Vital::Log::Type::Error);
            if (key.size() != static_cast<size_t>(EVP_CIPHER_key_length(algo))) throw Vital::Log::fetch("cipher-invalid-key", Vital::Log::Type::Error, std::string(key));
            if (iv.size() != static_cast<size_t>(EVP_CIPHER_iv_length(algo))) throw Vital::Log::fetch("cipher-invalid-iv", Vital::Log::Type::Error, std::string(iv));
            std::string out;
            int outLen1 = 0;
            int outLen2 = 0;
            out.resize(data.size() + EVP_CIPHER_block_size(algo));
            if (encrypt) {
                EVP_EncryptInit_ex(ctx, algo, nullptr, reinterpret_cast<const unsigned char*>(key.data()), reinterpret_cast<const unsigned char*>(iv.data()));
                EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(out.data()), &outLen1, reinterpret_cast<const unsigned char*>(data.data()), static_cast<int>(data.size()));
                EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(out.data()) + outLen1, &outLen2);
            }
            else {
                EVP_DecryptInit_ex(ctx, algo, nullptr, reinterpret_cast<const unsigned char*>(key.data()), reinterpret_cast<const unsigned char*>(iv.data()));
                EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(out.data()), &outLen1, reinterpret_cast<const unsigned char*>(data.data()), static_cast<int>(data.size()));
                EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(out.data()) + outLen1, &outLen2);
            }
            EVP_CIPHER_CTX_free(ctx);
            out.resize(outLen1 + outLen2);
            return out;
        }
    }

    inline std::string hash(std::string_view mode, std::string_view buffer) {
        const EVP_MD* md = Internal::hash_mode(mode);
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(ctx, md, nullptr);
        EVP_DigestUpdate(ctx, buffer.data(), buffer.size());
        unsigned char digest[EVP_MAX_MD_SIZE];
        unsigned int len = 0;
        EVP_DigestFinal_ex(ctx, digest, &len);
        EVP_MD_CTX_free(ctx);
        std::ostringstream ss;
        for (unsigned i = 0; i < len; ++i)
            ss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(digest[i]);
        return ss.str();
    }

    inline std::string encode(std::string_view buffer) {
        return Internal::base64_encode(buffer);
    }

    inline std::string decode(std::string_view buffer) {
        return Internal::base64_decode(buffer);
    }

    inline std::pair<std::string, std::string> encrypt(std::string_view mode, std::string_view buffer, std::string_view key) {
        const EVP_CIPHER* algo = Internal::cipher_mode(mode);
        std::string iv(EVP_CIPHER_iv_length(algo), '\0');
        RAND_bytes(reinterpret_cast<unsigned char*>(iv.data()), iv.size());
        std::string encrypted = Internal::cipher_run(algo, true, buffer, key, iv);
        return {
            Internal::base64_encode(encrypted),
            Internal::base64_encode(iv)
        };
    }

    inline std::string decrypt(std::string_view mode, std::string_view buffer, std::string_view key, std::string_view iv) {
        const EVP_CIPHER* algo = Internal::cipher_mode(mode);
        std::string raw = Internal::cipher_run(algo, false, Internal::base64_decode(buffer), key, Internal::base64_decode(iv));
        return raw;
    }
}