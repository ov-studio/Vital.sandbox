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
#include <System/public/file.h>
#include <System/public/crypto.h>
#include <Sandbox/lua/public/index.h>
#include <Sandbox/js/public/index.h>


//////////////
// Bundler //
//////////////

void outputConsole(std::string message) { std::cout << "\nVital.bundler | " << message; }
void genPackage(std::string name, std::string entry, std::vector<std::string> modules) {
    std::string rwBundle = "namespace Vital::Sandbox::" + name + " {\nstd::vector<std::string> vBundle = {";
    outputConsole("Packaging " + name + "...");
    for (auto i : modules) {
        std::string path = entry + i;
        if (!Vital::System::File::exists(path)) {
            outputConsole("Invalid File: " + path);
            throw 0;
        }
        else {
            rwBundle += "\nR\"" + Vital::vSignature + "(\n" + Vital::System::File::read(path) + "\n)" + Vital::vSignature + "\",";
            outputConsole("Bundled File: " + path);
        }
    }
    rwBundle += "\n};\n}";
    std::string path = entry + "bundle.h";
    Vital::System::File::write(path, rwBundle);
    outputConsole("Packaged " + name + " successfully!");
}

#include <Sandbox/lua/public/api.h>
#include <Sandbox/js/public/api.h>
#include <System/public/audio.h>


int main() {
    genPackage("Lua", "Sandbox/lua/module/", Vital::Sandbox::Lua::vModules);
    genPackage("JS", "Sandbox/js/module/", Vital::Sandbox::JS::vModules);
    /*
    Vital::Sandbox::Lua::API::boot();
    Vital::Sandbox::Lua::API::onErrorHandle([](std::string& err) -> void {
        std::cout << "\n" << err;
    });
    std::string rwString = R"(
        print("\n")
        local buffer = "test"
        print(crypto:decode(buffer))
    )";
    auto testVM = new Vital::Sandbox::Lua::create();
    testVM -> loadString(rwString);
    */

    Vital::Sandbox::JS::API::boot();
    Vital::Sandbox::JS::API::onErrorHandle([](std::string& err) -> void {
        std::cout << "\n" << err;
    });
    std::string rwString = R"(
    test = function() {
        return Duktape;
    }
    test()
    )";
    auto testVM = new Vital::Sandbox::JS::create();
    testVM -> loadString(rwString);
    std::string result = testVM -> getString(-1);
    std::cout << "\n" << "RESULT: " << result;
    Vital::System::Audio::create();
    return 1;
}
