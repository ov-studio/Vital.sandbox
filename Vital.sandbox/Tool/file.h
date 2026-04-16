/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: file.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: File Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <Vital.sandbox/Tool/log.h>
#include <Vital.sandbox/Tool/crypto.h>


////////////////////////
// Vital: Tool: File //
///////////////////////

namespace Vital::Tool::File {
    namespace Internal {
        inline godot::String normalize(const godot::String& path) {
            godot::String result = path.simplify_path();
            if (result == godot::String(".")) return godot::String();
            return result;
        }
    
        inline bool sanitize(const godot::String& path) {
            if (path.is_empty()) return false;
            if (path.begins_with("/") || path.begins_with("\\")) return false;
            if (path.find(":") != -1) return false;
            auto parts = path.replace("\\", "/").split("/");
            bool first = true;
            for (const auto& part : parts) {
                if (part.is_empty()) { first = false; continue; }
                bool all_dots = true;
                for (int i = 0; i < part.length(); i++) {
                    if (part[i] != '.') { all_dots = false; break; }
                }
                if (all_dots) {
                    if (first && part.length() == 1) { first = false; continue; }
                    return false;
                }
                first = false;
            }
            return true;
        }

        inline godot::Ref<godot::DirAccess> assert_base(const godot::String& path) {
            auto dir = godot::DirAccess::open(path);
            if (!dir.is_valid()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, fmt::format("\n> Reason: invalid base path `{}`", Tool::to_std_string(path)));
            return dir;
        }

        inline godot::String assert_path(const godot::String& path) {
            auto norm = normalize(path);
            if (!sanitize(norm)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, fmt::format("\n> Reason: invalid file path `{}`", Tool::to_std_string(path)));
            return norm;
        }

        inline std::tuple<godot::Ref<godot::DirAccess>, godot::String, godot::String> assert_base_and_path(const godot::String& base, const godot::String& path) {
            auto dir = assert_base(base);
            auto dest = assert_path(path);
            return {dir, dest, dir -> get_current_dir() + godot::String("/") + dest};
        }
    }

    inline bool sanitize(const godot::String& path) {
        return Internal::sanitize(path);
    }

    inline bool sanitize(const std::string& path) {
        return sanitize(Tool::to_godot_string(path));
    }

    inline bool exists(const godot::String& base, const godot::String& target) {
        auto [dir, dest, full_path] = Internal::assert_base_and_path(base, target);
        return dir.is_valid() && dir -> file_exists(dest);
    }

    inline bool exists(const std::string& base, const std::string& target) {
        return exists(Tool::to_godot_string(base), Tool::to_godot_string(target));
    }

    inline uint64_t size(const godot::String& base, const godot::String& target) {
        auto [dir, dest, full_path] = Internal::assert_base_and_path(base, target);
        if (!dir -> file_exists(dest)) throw Tool::Log::fetch("file-nonexistent", Tool::Log::Type::Error, Tool::to_std_string(target));
        auto file = godot::FileAccess::open(full_path, godot::FileAccess::READ);
        if (!file.is_valid()) throw Tool::Log::fetch("file-busy", Tool::Log::Type::Error, Tool::to_std_string(target));
        return file -> get_length();
    }

    inline uint64_t size(const std::string& base, const std::string& target) {
        return size(Tool::to_godot_string(base), Tool::to_godot_string(target));
    }

    inline std::string hash(const godot::String& base, const godot::String& target, std::string_view mode = "SHA256") {
        auto [dir, dest, full_path] = Internal::assert_base_and_path(base, target);
        if (!dir -> file_exists(dest)) throw Tool::Log::fetch("file-nonexistent", Tool::Log::Type::Error, Tool::to_std_string(target));
        return Tool::Crypto::hash_file(mode, Tool::to_std_string(full_path));
    }

    inline std::string hash(const std::string& base, const std::string& target, std::string_view mode = "SHA256") {
        return hash(Tool::to_godot_string(base), Tool::to_godot_string(target), mode);
    }

    inline bool remove(const godot::String& base, const godot::String& target) {
        auto [dir, dest, full_path] = Internal::assert_base_and_path(base, target);
        if (!dir -> file_exists(dest)) return false;
        return dir -> remove(dest) == godot::OK;
    }

    inline bool remove(const std::string& base, const std::string& target) {
        return remove(Tool::to_godot_string(base), Tool::to_godot_string(target));
    }

    inline std::string read_text(const godot::String& base, const godot::String& target) {
        auto [dir, dest, full_path] = Internal::assert_base_and_path(base, target);
        if (!dir -> file_exists(dest)) throw Tool::Log::fetch("file-nonexistent", Tool::Log::Type::Error, Tool::to_std_string(target));
        auto file = godot::FileAccess::open(full_path, godot::FileAccess::READ);
        if (!file.is_valid()) throw Tool::Log::fetch("file-busy", Tool::Log::Type::Error, Tool::to_std_string(target));
        return Tool::to_std_string(file -> get_as_text());
    }

    inline std::string read_text(const std::string& base, const std::string& target) {
        return read_text(Tool::to_godot_string(base), Tool::to_godot_string(target));
    }

    inline godot::PackedByteArray read_binary(const godot::String& base, const godot::String& target) {
        auto [dir, dest, full_path] = Internal::assert_base_and_path(base, target);
        if (!dir -> file_exists(dest)) throw Tool::Log::fetch("file-nonexistent", Tool::Log::Type::Error, Tool::to_std_string(target));
        auto file = godot::FileAccess::open(full_path, godot::FileAccess::READ);
        if (!file.is_valid()) throw Tool::Log::fetch("file-busy", Tool::Log::Type::Error, Tool::to_std_string(target));
        return file -> get_buffer(file -> get_length());
    }

    inline godot::PackedByteArray read_binary(const std::string& base, const std::string& target) {
        return read_binary(Tool::to_godot_string(base), Tool::to_godot_string(target));
    }

    inline bool write_text(const godot::String& base, const godot::String& target, const std::string& text) {
        godot::DirAccess::make_dir_recursive_absolute(base);
        auto [dir, dest, full_path] = Internal::assert_base_and_path(base, target);
        godot::DirAccess::make_dir_recursive_absolute(full_path.get_base_dir());
        auto file = godot::FileAccess::open(full_path, godot::FileAccess::WRITE);
        if (!file.is_valid()) throw Tool::Log::fetch("file-busy", Tool::Log::Type::Error, Tool::to_std_string(target));
        file -> store_string(Tool::to_godot_string(text));
        return true;
    }

    inline bool write_text(const std::string& base, const std::string& target, const std::string& text) {
        return write_text(Tool::to_godot_string(base), Tool::to_godot_string(target), text);
    }

    inline bool write_binary(const godot::String& base, const godot::String& target, const godot::PackedByteArray& data) {
        godot::DirAccess::make_dir_recursive_absolute(base);
        auto [dir, dest, full_path] = Internal::assert_base_and_path(base, target);
        godot::DirAccess::make_dir_recursive_absolute(full_path.get_base_dir());
        auto file = godot::FileAccess::open(full_path, godot::FileAccess::WRITE);
        if (!file.is_valid()) throw Tool::Log::fetch("file-busy", Tool::Log::Type::Error, Tool::to_std_string(target));
        file -> store_buffer(data);
        return true;
    }

    inline bool write_binary(const std::string& base, const std::string& target, const godot::PackedByteArray& data) {
        return write_binary(Tool::to_godot_string(base), Tool::to_godot_string(target), data);
    }

    inline std::vector<std::string> contents(const godot::String& base, const godot::String& target, bool directory_search = false) {
        auto [dir, dest, full_path] = Internal::assert_base_and_path(base, target);
        dir = godot::DirAccess::open(dest.is_empty() ? dir -> get_current_dir() : full_path);
        if (!dir.is_valid()) throw Tool::Log::fetch("directory-nonexistent", Tool::Log::Type::Error, Tool::to_std_string(target));
        std::vector<std::string> result;
        dir -> list_dir_begin();
        while (true) {
            auto name = dir -> get_next();
            if (name.is_empty()) break;
            if (name == "." || name == "..") continue;
            if (directory_search != dir -> current_is_dir()) continue;
            result.emplace_back(Tool::to_std_string(dest.is_empty() ? name : dest + godot::String("/") + name));
        }
        dir -> list_dir_end();
        return result;
    }

    inline std::vector<std::string> contents(const std::string& base, const std::string& target, bool directory_search = false) {
        return contents(Tool::to_godot_string(base), Tool::to_godot_string(target), directory_search);
    }

    inline std::vector<std::string> glob(const godot::String& base, const godot::String& pattern) {
        std::vector<std::string> result;
        auto pattern_norm = Internal::normalize(pattern);
        int last_slash = std::max(pattern_norm.rfind("/"), pattern_norm.rfind("\\"));
        auto dir_part = last_slash >= 0 ? pattern_norm.substr(0, last_slash) : godot::String();
        auto file_part = last_slash >= 0 ? pattern_norm.substr(last_slash + 1) : pattern_norm;
        bool has_recursive_wildcard = dir_part.find("**") != -1;
        bool has_single_wildcard = file_part.find("*") != -1;

        if (!has_recursive_wildcard && !has_single_wildcard) {
            auto full_path = dir_part.is_empty() ? file_part : (dir_part + godot::String("/") + file_part);
            if (exists(base, full_path)) result.emplace_back(Tool::to_std_string(full_path));
            return result;
        }

        auto matches_file = [&](const godot::String& name) {
            if (contains_wildcard(Tool::to_std_string(file_part))) return match_wildcard(Tool::to_std_string(file_part), Tool::to_std_string(name));
            return name == file_part;
        };

        if (has_recursive_wildcard) {
            auto search_base = dir_part.replace("**", godot::String());
            if (search_base.ends_with("/")) search_base = search_base.substr(0, search_base.length() - 1);
            std::function<void(const godot::String&)> walk = [&](const godot::String& current_dir) {
                auto full_dir = search_base.is_empty() ? current_dir : (search_base + godot::String("/") + current_dir);
                auto open_path = full_dir.is_empty() ? base : (base + godot::String("/") + full_dir);
                auto dir = godot::DirAccess::open(open_path);
                if (!dir.is_valid()) return;
                dir -> list_dir_begin();
                while (true) {
                    auto name = dir -> get_next();
                    if (name.is_empty()) break;
                    if (name == "." || name == "..") continue;
                    auto current_rel = current_dir.is_empty() ? name : (current_dir + godot::String("/") + name);
                    auto full_path = search_base.is_empty() ? current_rel : (search_base + godot::String("/") + current_rel);
                    if (dir -> current_is_dir()) walk(current_rel);
                    else if (matches_file(name)) result.emplace_back(Tool::to_std_string(full_path));
                }
                dir -> list_dir_end();
            };
            walk(godot::String());
        }
        else {
            auto open_path = dir_part.is_empty() ? base : (base + godot::String("/") + dir_part);
            auto dir = godot::DirAccess::open(open_path);
            if (dir.is_valid()) {
                dir -> list_dir_begin();
                while (true) {
                    auto name = dir -> get_next();
                    if (name.is_empty()) break;
                    if (name == "." || name == "..") continue;
                    if (!dir -> current_is_dir() && matches_file(name)) {
                        auto full_path = dir_part.is_empty() ? name : (dir_part + godot::String("/") + name);
                        result.emplace_back(Tool::to_std_string(full_path));
                    }
                }
                dir -> list_dir_end();
            }
        }
        return result;
    }

    inline std::vector<std::string> glob(const std::string& base, const std::string& pattern) {
        return glob(Tool::to_godot_string(base), Tool::to_godot_string(pattern));
    }
}