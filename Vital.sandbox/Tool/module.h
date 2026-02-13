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

    inline std::unordered_map<std::string, std::string> module_cache;
    inline std::mutex module_mutex;

    inline std::vector<std::string> get_modules(const std::string& name) {
        std::vector<std::string> result;
        std::lock_guard<std::mutex> lock(module_mutex);
        const std::string url = fmt::format(Repo_Kit, "manifest.json");
        if (module_cache.find(url) == module_cache.end()) module_cache[url] = Vital::Tool::Rest::get(url);        
        rapidjson::Document document;
        document.Parse(module_cache[url].c_str());
        if (!document.HasParseError() && document.HasMember(name.c_str())) {
            for (auto& i : document[name.c_str()]["sources"].GetArray()) {
                std::string source_name = i.GetString();
                std::string url = fmt::format(Repo_Kit, name + "/" + source_name);
                if (module_cache.find(url) == module_cache.end()) module_cache[url] = Vital::Tool::Rest::get(url);                
                result.push_back(module_cache[url]);
            }
        }
        return result;
    }
}