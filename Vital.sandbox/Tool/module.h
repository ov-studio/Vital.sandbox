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
    inline std::mutex module_mutex;
    inline std::unordered_map<std::string, std::string> module_cache;

    inline const std::string& fetch_content(std::string_view url) {
        std::lock_guard<std::mutex> lock(module_mutex);
        std::string request(url);
        auto it = module_cache.find(request);
        if (it == module_cache.end()) {
            it = module_cache.emplace(std::move(request), Vital::Tool::Rest::get(std::string(url))).first;
        }
        return it -> second;
    }
    
    template<typename... Keys>
    inline Vital::Tool::StackValue fetch_config(const std::string& name, Keys&&... keys) {
        rapidjson::Document document;
        document.Parse(fetch_content(fmt::format(Repo_Kit, "manifest.json")).c_str());
        if (document.HasParseError() || !document.HasMember(name.c_str())) return {};
        const rapidjson::Value* node = &document[name.c_str()];
        auto to_key = [](auto&& k) -> std::string {
            using T = std::decay_t<decltype(k)>;
            if constexpr (std::is_integral_v<T>) return std::to_string(k);
            else return std::string(k);
        };
        for (const std::string& key : {to_key(std::forward<Keys>(keys))...}) {
            bool is_index = !key.empty() && std::all_of(key.begin(), key.end(), ::isdigit);
            if (is_index) {
                if (!node -> IsArray()) return {};
                const auto index = static_cast<rapidjson::SizeType>(std::stoul(key));
                if (index >= node -> Size()) return {};
                node = &(*node)[index];
            }
            else {
                if (!node -> IsObject() || !node -> HasMember(key.c_str())) return {};
                node = &(*node)[key.c_str()];
            }
        }
        if (node -> IsString()) return std::string(node -> GetString());
        else if (node -> IsInt()) return node -> GetInt();
        else if (node -> IsInt64()) return node -> GetInt64();
        else if (node -> IsFloat()) return node -> GetFloat();
        else if (node -> IsDouble()) return node -> GetDouble();
        else if (node -> IsBool()) return node -> GetBool();
        return {};
    }

    inline std::string fetch_module(const std::string& name) {
        const rapidjson::Value* node = fetch_config_base(name);
        if (!node || !node -> HasMember("source")) return "";
        return fetch_content(fmt::format(Repo_Kit, name + "/" + (*node)["source"].GetString()));
    }

    inline std::vector<std::string> fetch_modules(const std::string& name) {
        std::vector<std::string> result;
        const rapidjson::Value* node = fetch_config_base(name);
        if (!node || !node -> HasMember("sources") || !(*node)["sources"].IsArray()) return result;
        for (auto& i : (*node)["sources"].GetArray()) {
            result.push_back(fetch_content(fmt::format(Repo_Kit, name + "/" + std::string(i.GetString()))));
        }
        return result;
    }
}