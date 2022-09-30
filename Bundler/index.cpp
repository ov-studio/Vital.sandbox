/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Bundler: index.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Bundler Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Sandbox/lua/public/index.h>
#include <System/public/filesystem.h>

#include <System/public/crypto.h>


//////////////
// Bundler //
//////////////

void outputConsole(std::string message) { std::cout << "\nVital.bundler | " << message; }
void genPackage(std::string name, std::string entry, std::vector<std::string> modules) {
    std::string rwBundle = "namespace Vital::" + name + " {\nstd::vector<std::string> vBundle = {";
    outputConsole("Packaging " + name + "...");
    for (auto i : modules) {
        std::string path = entry + i;
        if (!Vital::FileSystem::exists(path)) {
            outputConsole("Invalid File: " + path);
            throw 0;
        }
        else {
            rwBundle += "\nR\"" + Vital::vSignature + "(\n" + Vital::FileSystem::read(path) + "\n)" + Vital::vSignature + "\",";
            outputConsole("Bundled File: " + path);
        }
    }
    rwBundle += "\n};\n}";
    std::string path = entry + "bundle.h";
    Vital::FileSystem::write(path, rwBundle);
    outputConsole("Packaged " + name + " successfully!");
}

#include <Sandbox/lua/public/api.h>

int main() {
    genPackage("Lua", "Sandbox/lua/module/", Vital::Lua::vModules);

    /*
    std::string buffer = "xdddd";
    std::string key = "01234567890123456789012345678901";
    auto encResult = Vital::Crypto::encrypt("AES256", buffer, key);
    std::cout << "\nENCRYPTED VALUE: " << encResult.first << " IV: " << encResult.second;
    auto decResult = Vital::Crypto::decrypt("AES256", encResult.first, key, encResult.second);
    std::cout << "\nDECRYPTED VALUE: " << decResult;
    */
    Vital::Lua::API::boot();
    Vital::Lua::API::onErrorHandle([](std::string& err) -> void {
        std::cout << "\n" << err;
    });
    std::string rwString = R"(
        print("\n")
        
        print(crypto:decode("123"))
        --[[
        print("SHA1 (Hash): "..crypto:hash("SHA1", "test"))
        print("SHA224 (Hash): "..crypto:hash("SHA224", "test"))
        print("SHA256 (Hash): "..crypto:hash("SHA256", "test"))
        print("SHA384 (Hash): "..crypto:hash("SHA384", "test"))
        print("SHA512 (Hash): "..crypto:hash("SHA512", "test"))

        local buffer, key = "EncryptMe", "0123456789012345"
        local hash, iv = crypto:encrypt("AES128", buffer, key)
        print("AES128 (Encrypt): "..hash)
        print("AES128 (Decrypt): "..crypto:decrypt("AES128", hash, key, iv))

        local buffer, key = "EncryptMeEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEETEST.....WORKS??YESCHECKAGAIN;YESITDOES", "012345678901234511111111"
        local hash, iv = crypto:encrypt("AES192", buffer, key)
        print("AES192 (Encrypt): "..hash.." | Length: "..#hash)
        print("AES192 (Decrypt): "..crypto:decrypt("AES192", hash, key, iv))

        local buffer, key = "EncryptMe", "01234567890123456789012345678901"
        local hash, iv = crypto:encrypt("AES256", buffer, key)
        print("AES256 (Encrypt): "..hash)
        print("AES256 (Decrypt): "..crypto:decrypt("AES256", hash, key, iv))
        ]]
    )";
    auto testVM = new Vital::Lua::create();
    testVM -> loadString(rwString);
    return 1;
}