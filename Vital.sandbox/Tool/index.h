/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: public: index.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Root Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <pch.h>
#include <Vital.sandbox/System/public/rest.h>
#include <Vital.sandbox/Vendor/rapidjson/document.h>


////////////
// Vital //
////////////

namespace Vital {
    namespace Util {}
    namespace System {}
    namespace Sandbox {}
    static const std::string Signature = "vsdk_v.0.0.1";
    static const std::string Repository = "https://raw.githubusercontent.com/ov-studio/Vital.sandbox/refs/heads/module/{}";

    static inline std::vector<std::string> get_modules(const std::string& name) {
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