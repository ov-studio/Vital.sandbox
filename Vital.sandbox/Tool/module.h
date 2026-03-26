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
    inline std::mutex content_mutex;
    inline std::unordered_map<std::string, std::string> content_cache;
    inline std::unordered_map<std::string, rapidjson::Document> json_cache;
    static const std::string toolkit = "https://raw.githubusercontent.com/ov-studio/Vital.kit/refs/heads/main/{}";

    inline const std::string& fetch_content(std::string_view url) {
        std::lock_guard<std::mutex> lock(content_mutex);
        std::string request(url);
        auto it = content_cache.find(request);
        if (it == content_cache.end()) {
            it = content_cache.emplace(std::move(request), Vital::Tool::Rest::get(std::string(url))).first;
        }
        return it -> second;
    }

    inline rapidjson::Document& fetch_json(const std::string& name) {
        auto it = json_cache.find(name);
        if (it == json_cache.end()) {
            rapidjson::Document document;
            document.Parse(fetch_content(fmt::format(toolkit, name + ".json")).c_str());
            it = json_cache.emplace(name, std::move(document)).first;
        }
        return it -> second;
    }

    inline const rapidjson::Value* fetch_json_node(const std::string& name, const std::string& key) {
        auto& document = fetch_json(name);
        if (document.HasParseError() || !document.HasMember(key.c_str())) return nullptr;
        const auto& entry = document[key.c_str()];
        if (!entry.IsObject()) return nullptr;
        return &entry;
    }

    template<typename... Keys>
    inline Vital::Tool::StackValue fetch_json_value(const std::string& name, Keys&&... keys) {
        auto& document = fetch_json(name);
        if (document.HasParseError() || !document.IsObject()) return {};
        const rapidjson::Value* node = &document;
        std::vector<std::string> key_list;
        (key_list.push_back([](auto&& k) -> std::string {
            using T = std::decay_t<decltype(k)>;
            if constexpr (std::is_integral_v<T>) return std::to_string(k);
            else return std::string(k);
        }(std::forward<Keys>(keys))), ...);
        for (const std::string& key : key_list) {
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
        auto& document = fetch_json(name + "/manifest");
        if (document.HasParseError() || !document.HasMember("source")) return "";
        return fetch_content(fmt::format(toolkit, name + "/" + document["source"].GetString()));
    }

    inline std::vector<std::string> fetch_modules(const std::string& name) {
        std::vector<std::string> result;
        auto& document = fetch_json(name + "/manifest");
        if (document.HasParseError() || !document.HasMember("sources") || !document["sources"].IsArray()) return result;
        for (auto& i : document["sources"].GetArray()) {
            result.push_back(fetch_content(fmt::format(toolkit, name + "/" + std::string(i.GetString()))));
        }
        return result;
    }
}