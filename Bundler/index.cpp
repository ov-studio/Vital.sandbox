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
#include <System/public/filesystem.h>
#include <System/public/crypto.h>
#include <Sandbox/lua/public/index.h>
#include <Sandbox/js/public/index.h>


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
    genPackage("JS", "Sandbox/js/module/", Vital::JS::vModules);
    /*
    Vital::Lua::API::boot();
    Vital::Lua::API::onErrorHandle([](std::string& err) -> void {
        std::cout << "\n" << err;
    });
    std::string rwString = R"(
        print("\n")
        local buffer = "test"
        print(crypto:decode(buffer))
    )";
    auto testVM = new Vital::Lua::create();
    testVM -> loadString(rwString);
    */

    Vital::JS::test();
    return 1;
}
