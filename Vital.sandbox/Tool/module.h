/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: module.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Module Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <Vital.sandbox/Tool/rest.h>
#include <Vital.sandbox/Vendor/rapidjson/document.h>


////////////
// Vital //
////////////

namespace Vital::Tool {
    class Module {
        public:
            static void bind(void* entity) {};
            static void inject(void* entity) {};
    };

    inline std::vector<std::string> get_modules(const std::string& name) {
        std::vector<std::string> result;
        rapidjson::Document manifest;
        manifest.Parse(Vital::System::Rest::get(fmt::format(Repository, "manifest.json")).c_str());
        if (!manifest.HasParseError() && manifest.HasMember(name.c_str())) {
            for (auto& i : manifest[name.c_str()]["sources"].GetArray()) {
                result.push_back(Vital::System::Rest::get(fmt::format(Repository, name + "/" + std::string(i.GetString()))));
            }
        }
        return result;
    }
}