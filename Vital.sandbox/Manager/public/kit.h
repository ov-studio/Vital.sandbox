/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: kit.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Kit Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <Vital.sandbox/Tool/rest.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <zip.h>


//////////////////////////
// Vital: Manager: Kit //
//////////////////////////

namespace Vital::Manager::Kit {
    inline constexpr std::string_view toolkit_api = "https://api.github.com/repos/ov-studio/Vital.kit/releases/latest";
    inline constexpr std::string_view cache_base = "cache";
    inline constexpr std::string_view kit_name = "Vital.kit";
    inline std::mutex content_mutex;
    inline std::unordered_map<std::string, std::string> content_cache;
    inline std::unordered_map<std::string, rapidjson::Document> json_cache;
    inline const Tool::Rest::rest_headers kit_headers = { "User-Agent: Vital.sandbox" };
    inline std::string version;

    std::tuple<std::string, std::string, std::string> fetch_release_info();
    rapidjson::Document fetch_checksum(const std::string& checksum_url, std::string& checksum_hash);
    bool extract_zip(const std::string& zip_path, const std::string& dest_dir);
    bool download_file(const std::string& url, const std::string& dest_path);
    bool ensure_kit();

    inline const std::string& get_version() {
        if (!version.empty()) return version;
        const std::string checksum_path = std::string(cache_base) + "/" + std::string(kit_name) + "/checksum.json";
        std::ifstream file(checksum_path, std::ios::binary);
        if (!file) return version;
        std::string raw{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
        rapidjson::Document doc;
        doc.Parse(raw.c_str());
        if (doc.HasParseError() || !doc.IsObject()) return version;
        if (doc.HasMember("version") && doc["version"].IsString()) version = doc["version"].GetString();
        return version;
    }

    inline std::string fetch_file(const std::string& rel_path) {
        std::filesystem::path full = std::filesystem::path(cache_base)/kit_name/rel_path;
        std::ifstream file(full, std::ios::binary);
        if (!file) return {};
        return { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
    }

    inline const std::string& fetch_content(std::string_view path) {
        std::lock_guard<std::mutex> lock(content_mutex);
        std::string key(path);
        auto it = content_cache.find(key);
        if (it != content_cache.end()) return it -> second;
        std::string value = fetch_file(key);
        if (value.empty()) {
            static const std::string empty{};
            return empty;
        }
        return content_cache.emplace(key, std::move(value)).first -> second;
    }

    inline rapidjson::Document& fetch_json(const std::string& name) {
        auto it = json_cache.find(name);
        if (it != json_cache.end()) return it -> second;
        rapidjson::Document document;
        document.Parse(fetch_content(name + ".json").c_str());
        if (document.HasParseError()) {
            static rapidjson::Document empty_doc;
            return empty_doc;
        }
        return json_cache.emplace(name, std::move(document)).first -> second;
    }

    template<typename... Keys>
    inline const rapidjson::Value* fetch_json_detail(const rapidjson::Value* node, Keys&&... keys) {
        std::vector<std::string> key_list;
        (key_list.push_back([](auto&& k) -> std::string {
            using T = std::decay_t<decltype(k)>;
            if constexpr (std::is_integral_v<T>) return std::to_string(k);
            else return std::string(k);
        }(std::forward<Keys>(keys))), ...);
        for (const std::string& key : key_list) {
            bool is_index = !key.empty() && std::all_of(key.begin(), key.end(), ::isdigit);
            if (is_index) {
                if (!node -> IsArray()) return nullptr;
                const auto index = static_cast<rapidjson::SizeType>(std::stoul(key));
                if (index >= node -> Size()) return nullptr;
                node = &(*node)[index];
            }
            else {
                if (!node -> IsObject() || !node -> HasMember(key.c_str())) return nullptr;
                node = &(*node)[key.c_str()];
            }
        }
        return node;
    }

    template<typename... Keys>
    inline const rapidjson::Value* fetch_json_node(const std::string& name, Keys&&... keys) {
        auto& document = fetch_json(name);
        if (document.HasParseError() || !document.IsObject()) return nullptr;
        const rapidjson::Value* node = fetch_json_detail(&document, std::forward<Keys>(keys)...);
        if (!node || (!node -> IsObject() && !node -> IsArray())) return nullptr;
        return node;
    }

    template<typename... Keys>
    inline Tool::StackValue fetch_json_value(const std::string& name, Keys&&... keys) {
        auto& document = fetch_json(name);
        if (document.HasParseError() || !document.IsObject()) return {};
        const rapidjson::Value* node = fetch_json_detail(&document, std::forward<Keys>(keys)...);
        if (!node) return {};
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
        return std::string(fetch_content(name + "/" + document["source"].GetString()));
    }

    inline std::vector<std::string> fetch_modules(const std::string& name) {
        std::vector<std::string> result;
        auto& document = fetch_json(name + "/manifest");
        if (document.HasParseError() || !document.HasMember("sources") || !document["sources"].IsArray()) return result;
        for (auto& i : document["sources"].GetArray())
            result.push_back(std::string(fetch_content(name + "/" + std::string(i.GetString()))));
        return result;
    }
}