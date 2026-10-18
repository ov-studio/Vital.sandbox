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
#include <Vital.sandbox/Engine/public/core.h>


//////////////////////////
// Vital: Manager: Kit //
//////////////////////////

namespace Vital::Manager {
    class Kit : public godot::Node, public Tool::Base<Kit> {
        friend class Tool::Base<Kit>;
        public:
            static constexpr const char* Name = "Kit.manager";
        private:
            static constexpr std::string_view toolkit_src = "https://api.github.com/repos/ov-studio/Vital.kit/releases/latest";
            static constexpr std::string_view toolkit_base = "cache";
            static constexpr std::string_view toolkit_name = "Vital.kit";
            static constexpr size_t download_max_size = 1024ull * 1024 * 1024;
            static constexpr zip_uint64_t extract_max_ratio = 200;
            static constexpr zip_int64_t extract_max_entries = 100000;
            static constexpr zip_uint64_t extract_max_entry_size = 2ull * 1024 * 1024 * 1024;
            static constexpr zip_uint64_t extract_max_total_size = 4ull * 1024 * 1024 * 1024;
            mutable std::mutex mutex;
            std::string version = "";
            std::unordered_map<std::string, std::string> content_cache;
            std::unordered_map<std::string, rapidjson::Document> json_cache;

            struct Internal {
                // Helpers //
                static std::string to_lower(std::string value);
                static const std::string& get_version();
                static const std::string& get_content(std::string_view path);
                static rapidjson::Document& get_json(const std::string& name);
                static bool resolve_entry(const std::filesystem::path& root, const std::string& entry, std::filesystem::path& out);
                static bool verify_tree(const std::filesystem::path& root, const rapidjson::Document& checksum_doc, const std::string& remote_hash);

                template<typename... Keys>
                static const rapidjson::Value* get_json_detail(const rapidjson::Value* node, Keys&&... keys) {
                    auto to_key = [](auto&& k) -> std::string {
                        using T = std::decay_t<decltype(k)>;
                        if constexpr (std::is_integral_v<T>) return std::to_string(k);
                        else return std::string(k);
                    };
                
                    std::vector<std::string> key_list;
                    (key_list.push_back(to_key(std::forward<Keys>(keys))), ...);
                    for (const std::string& key : key_list) {
                        bool is_idx = !key.empty() && std::all_of(key.begin(), key.end(), ::isdigit);
                        if (is_idx) {
                            if (!node -> IsArray()) return nullptr;
                            const auto idx = static_cast<rapidjson::SizeType>(std::stoul(key));
                            if (idx >= node -> Size()) return nullptr;
                            node = &(*node)[idx];
                        }
                        else {
                            if (!node -> IsObject() || !node -> HasMember(key.c_str())) return nullptr;
                            node = &(*node)[key.c_str()];
                        }
                    }
                    return node;
                }
            };


            // Instantiators //
            Kit() = default;
            ~Kit() = default;
        public:
            // Getters //
            const std::string& get_version();
            const std::string& get_content(std::string_view path);
            rapidjson::Document& get_json(const std::string& name);
            std::vector<std::pair<std::string, std::string>> get_module(const std::string& name);
            std::tuple<std::string, std::string, std::string> get_release() const;
            rapidjson::Document get_checksum(const std::string& checksum_url, std::string& checksum_hash) const;

            template<typename... Keys>
            const rapidjson::Value* get_json_node(const std::string& name, Keys&&... keys) {
                std::lock_guard<std::mutex> lock(mutex);
                auto& document = Internal::get_json(name);
                if (document.HasParseError() || !document.IsObject()) return nullptr;
                const rapidjson::Value* node = Internal::get_json_detail(&document, std::forward<Keys>(keys)...);
                if (!node || (!node -> IsObject() && !node -> IsArray())) return nullptr;
                return node;
            }

            template<typename... Keys>
            Tool::StackValue get_json_value(const std::string& name, Keys&&... keys) {
                std::lock_guard<std::mutex> lock(mutex);
                auto& document = Internal::get_json(name);
                if (document.HasParseError() || !document.IsObject()) return {};
                const rapidjson::Value* node = Internal::get_json_detail(&document, std::forward<Keys>(keys)...);
                if (!node) return {};
                if (node -> IsString()) return std::string(node -> GetString());
                else if (node -> IsInt()) return node -> GetInt();
                else if (node -> IsInt64()) return node -> GetInt64();
                else if (node -> IsFloat()) return node -> GetFloat();
                else if (node -> IsDouble()) return node -> GetDouble();
                else if (node -> IsBool()) return node -> GetBool();
                return {};
            }


            // Misc //
            bool download(const std::string& url, const std::string& dest_path) const;
            bool extract(const std::string& zip_path, const std::string& dest_dir) const;
            bool ensure();
    };
}