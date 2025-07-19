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
        result += "\n\tstd::vector<std::string> buffer;";
        if (isDebugMode) std::cout << "\nPackaging: " << path;
        for (auto& i : modules) {
            if (!Vital::System::File::exists(i.path)) {
                if (isDebugMode) std::cout << "\nInvalid Module: " << i.path;
                return false;
            }
            else {
                result += "\n\tif (path == \"" + i.identifier + "\") {";
                result += Vital::System::Package::Module::write(Vital::System::File::read(i.path));
                result += "\n\t}";
                if (isDebugMode) std::cout << "\nBundled Module: " << i.path;
            }
        }
        result += "\n\treturn Vital::System::Package::Module::read(buffer);";
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

    std::string write(const std::string& buffer) {
        std::string result = "";
        const size_t inputLimit = 16380 - 1;
        size_t start = 0;
        size_t bufferSize = buffer.size();
        while (start < bufferSize) {
            size_t maxSize = std::min(bufferSize - start, inputLimit);
            std::string chunk = buffer.substr(start, maxSize);
            std::string encodedChunk = Vital::System::Crypto::encode(chunk);
            while (encodedChunk.size() > inputLimit && maxSize > 0) {
                maxSize--;
                chunk = buffer.substr(start, maxSize);
                encodedChunk = Vital::System::Crypto::encode(chunk);
            }
            result += "\n\t\tbuffer.push_back(\"" + encodedChunk + "\");";
            start += maxSize;
        }
        return result;
    }
}
