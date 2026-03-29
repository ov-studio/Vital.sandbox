/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: module.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Module Tools
----------------------------------------------------------------*/

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <Vital.sandbox/Tool/rest.h>
#include <Vital.sandbox/Tool/file.h>
#include <Vital.sandbox/Vendor/rapidjson/document.h>
#include <Vital.sandbox/Vendor/rapidjson/writer.h>
#include <Vital.sandbox/Vendor/rapidjson/stringbuffer.h>
#include <zip.h>

namespace Vital::Tool {
    // TODO: Improve
    extern std::mutex content_mutex;
    extern std::unordered_map<std::string, std::string> content_cache;
    extern std::unordered_map<std::string, rapidjson::Document> json_cache;
    extern const std::string toolkit_api;
    extern const std::string cache_base;
    extern const std::string kit_name;

    std::string read_kit_file(const std::string& rel_path);
    const std::string& fetch_content(std::string_view path);
    rapidjson::Document& fetch_json(const std::string& name);
    const rapidjson::Value* fetch_json_node(const std::string& name, const std::string& key);
    std::string fetch_module(const std::string& name);
    std::vector<std::string> fetch_modules(const std::string& name);

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
                if (!node->IsArray()) return {};
                const auto index = static_cast<rapidjson::SizeType>(std::stoul(key));
                if (index >= node->Size()) return {};
                node = &(*node)[index];
            } else {
                if (!node->IsObject() || !node->HasMember(key.c_str())) return {};
                node = &(*node)[key.c_str()];
            }
        }
        if (node->IsString())      return std::string(node->GetString());
        else if (node->IsInt())    return node->GetInt();
        else if (node->IsInt64())  return node->GetInt64();
        else if (node->IsFloat())  return node->GetFloat();
        else if (node->IsDouble()) return node->GetDouble();
        else if (node->IsBool())   return node->GetBool();
        return {};
    }

    namespace Kit {
        std::tuple<std::string, std::string, std::string> fetch_release_info();
        rapidjson::Document fetch_checksum(const std::string& checksum_url);
        bool extract_zip(const std::string& zip_path, const std::string& dest_dir);
        bool download_file(const std::string& url, const std::string& dest_path);
        bool ensure_kit();
    }
}