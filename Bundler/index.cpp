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

int main() {
    std::cout << "Hey the SHA of testing: ";
    Vital::sha256("testing");
    //genPackage("Lua", "Sandbox/lua/module/", Vital::Lua::vModules);
    return 1;
}