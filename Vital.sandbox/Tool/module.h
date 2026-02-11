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
#include <Vital.sandbox/Vendor/rapidjson/writer.h>
#include <Vital.sandbox/Vendor/rapidjson/stringbuffer.h>


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
        rapidjson::Document document;
        document.Parse(Vital::Tool::Rest::get(fmt::format(Repo_Kit, "manifest.json")).c_str());
        if (!document.HasParseError() && document.HasMember(name.c_str())) {
            for (auto& i : document[name.c_str()]["sources"].GetArray()) {
                result.push_back(Vital::Tool::Rest::get(fmt::format(Repo_Kit, name + "/" + std::string(i.GetString()))));
            }
        }
        return result;
    }
}