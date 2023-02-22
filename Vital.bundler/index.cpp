/*----------------------------------------------------------------
     Resource: Vital.bundler
     Script: index.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Bundler Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/file.h>
#include <Sandbox/lua/public/index.h>
#include <Sandbox/js/public/index.h>


//////////////
// Bundler //
//////////////

void outputConsole(const std::string& message) { std::cout << "\nVital.bundler | " << message; }
void genPackage(const std::string& name, const std::string& entry, std::vector<std::string> modules) {
    std::string rwBundle = "namespace Vital::Sandbox::" + name + " {\nstd::vector<std::string> rwBundle = {";
    outputConsole("Packaging " + name + "...");
    for (auto i : modules) {
        std::string path = entry + i;
        if (!Vital::System::File::exists(path)) {
            outputConsole("Invalid File: " + path);
            throw 0;
        }
        else {
            rwBundle += "\nR\"" + Vital::Signature + "(\n" + Vital::System::File::read(path) + "\n)" + Vital::Signature + "\",";
            outputConsole("Bundled File: " + path);
        }
    }
    rwBundle += "\n};\n}";
    std::string path = entry + "bundle.h";
    Vital::System::File::write(path, rwBundle);
    outputConsole("Packaged " + name + " successfully!");
}

int main() {
    genPackage("Lua", "../Vital.sandbox/Sandbox/lua/module/", Vital::Sandbox::Lua::module);
    genPackage("JS", "../Vital.sandbox/Sandbox/js/module/", Vital::Sandbox::JS::module);
    return 1;
}
