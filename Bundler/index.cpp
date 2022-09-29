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

    std::string buffer = "xdddd";
    std::string key = "01234567890123456789012345678901";
    Vital::Crypto::encrypt(buffer, key);
    /*
    Vital::Lua::API::boot();
    Vital::Lua::API::onErrorHandle([](std::string& err) -> void {
        std::cout << "\n" << err;
    });
    std::string rwString = R"(
        print("\n")
        print("SHA1: "..crypto:sha1("test"))
        print("SHA224: "..crypto:sha224("test"))
        print("SHA256: "..crypto:sha256("test"))
        print("SHA384: "..crypto:sha384("test"))
        print("SHA512: "..crypto:sha512("test"))
    )";
    auto testVM = new Vital::Lua::create();
    testVM -> loadString(rwString);
    */
    return 1;
}