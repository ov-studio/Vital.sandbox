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
    struct Module {
        static void bind(void* entity) {}
        static void inject(void* entity) {}
    };

    inline std::mutex module_mutex;
    inline std::unordered_map<std::string, std::string> module_cache;

    inline std::string fetch_content(const std::string& url) {
        std::lock_guard<std::mutex> lock(module_mutex);
        if (module_cache.find(url) == module_cache.end()) module_cache[url] = Vital::Tool::Rest::get(url);
        return module_cache[url];
    }
    
    inline std::vector<std::string> get_modules(const std::string& name) {
        std::vector<std::string> result;
        rapidjson::Document document;
        document.Parse(fetch_content(fmt::format(Repo_Kit, "manifest.json")).c_str());
        if (!document.HasParseError() && document.HasMember(name.c_str())) {
            for (auto& i : document[name.c_str()]["sources"].GetArray()) {
                std::string source_name = i.GetString();
                result.push_back(fetch_content(fmt::format(Repo_Kit, name + "/" + source_name)));
            }
        }
        return result;
    }
}