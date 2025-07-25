/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: private: error.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Error Codes
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/rest.h>
#include <Vendor/rapidjson/document.h>


/////////////////////
// Vital: Sandbox //
/////////////////////

namespace Vital::Sandbox {
    std::string repository = "https://raw.githubusercontent.com/ov-studio/Vital.sandbox/refs/heads/module/{}";

    std::vector<std::string> fetchModules(const std::string& name) {
        std::vector<std::string> result;
        rapidjson::Document manifest;
        manifest.Parse(Vital::System::REST::get(fmt::format(repository, "manifest.json")).c_str());
        if (!manifest.HasParseError() && manifest.HasMember(name.c_str())) {
            for (auto& i : manifest[name.c_str()]["sources"].GetArray()) {
                result.push_back(Vital::System::REST::get(fmt::format(repository, name + "/" + std::string(i.GetString()))));
            }
        }
        return result;
    }
}