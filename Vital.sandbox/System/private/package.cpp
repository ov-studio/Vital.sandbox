/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: private: package.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Package System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/package.h>
#include <System/public/crypto.h>
#include <System/public/file.h>


/////////////////////////////
// Vital: System: Package //
/////////////////////////////

namespace Vital::System::Package {
    bool create(std::string& path, std::vector<Vital::Type::Package::Module> modules, bool isDebugMode) {
        Vital::System::File::resolve(path);
        std::string result = "\n#pragma once";
        result += "\n#include <System/public/package.h>";
        result += "\nstd::string fetchPackageModule(const std::string& path) {";
        result += "\nstd::string result;";
        result += "\nstd::vector<std::string> module;";
        if (isDebugMode) std::cout << "\nPackaging: " << path;
        for (auto& i : modules) {
            if (!Vital::System::File::exists(i.path)) {
                if (isDebugMode) std::cout << "\nInvalid Module: " << i.path;
                return false;
            }
            else {
                result += "\nif (path == \"" + i.identifier + "\") {\n";
                result += Vital::System::Package::Module::write(Vital::System::File::read(i.path), i.delimiter);
                result += "module = buffer;";
                result += "\n}";
                if (isDebugMode) std::cout << "\nBundled Module: " << i.path;
            }
        }
        result += "\nreturn Vital::System::Package::Module::read(module);";
        result += "\n}";
        Vital::System::File::write(path, result);
        if (isDebugMode) std::cout << "\nPackaged: " << path;
        return true;
    }
}


/////////////////////////////////////
// Vital: System: Package: Module //
/////////////////////////////////////

namespace Vital::System::Package::Module {
    std::string read(std::vector<std::string> buffer) {
        std::string result;
        for (const auto& i : buffer) {
            result += Vital::System::Crypto::decode(i);
        }
        return result;
    }

    std::string write(const std::string& buffer, const char delimiter) {
        std::string result = "std::vector<std::string> buffer;";
        std::string chunk;
        std::stringstream stream(buffer);
        while (std::getline(stream, chunk, delimiter)) {
            result += "\nbuffer.push_back(\"" + Vital::System::Crypto::encode(chunk + delimiter) + "\");";
        }
        return result;
    }
}
