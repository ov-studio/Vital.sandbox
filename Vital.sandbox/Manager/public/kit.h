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
#include <Vital.sandbox/Tool/file.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <zip.h>


//////////////////////////
// Vital: Manager: Kit //
//////////////////////////

namespace Vital::Manager::Kit {
    extern const std::string toolkit_api;
    extern const std::string cache_base;
    extern const std::string kit_name;
    extern std::mutex content_mutex;
    extern std::unordered_map<std::string, std::string> content_cache;
    extern std::unordered_map<std::string, rapidjson::Document> json_cache;

    const std::string& get_version();
    std::tuple<std::string, std::string, std::string> fetch_release_info();
    rapidjson::Document fetch_checksum(const std::string& checksum_url, std::string& out_remote_hash);
    bool extract_zip(const std::string& zip_path, const std::string& dest_dir);
    bool download_file(const std::string& url, const std::string& dest_path);
    bool ensure_kit();

    std::string fetch_file(const std::string& rel_path);
    const std::string& fetch_content(std::string_view path);
    rapidjson::Document& fetch_json(const std::string& name);
    std::string fetch_module(const std::string& name);
    std::vector<std::string> fetch_modules(const std::string& name);

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
}