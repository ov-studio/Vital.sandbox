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
#include <System/public/package.h>
#include <Sandbox/lua/public/index.h>
#include <Sandbox/js/public/index.h>


//////////////
// Bundler //
//////////////

void createPackage(const std::string& entry, std::vector<std::string> modules) {
    std::vector<Vital::Type::Package::Module> rwModules;
    for (const auto& i : modules) {
        rwModules.push_back({i, entry + i, '\n'});
    }
    Vital::System::Package::create(entry + "bundle.h", rwModules, true);
}

int main() {
    createPackage("../Vital.sandbox/Sandbox/lua/module/", Vital::Sandbox::Lua::vsdk_modules);
    createPackage("../Vital.sandbox/Sandbox/js/module/", Vital::Sandbox::JS::vsdk_modules);
    return 1;
}
