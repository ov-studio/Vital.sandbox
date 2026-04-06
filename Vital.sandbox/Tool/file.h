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
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>


////////////////////////
// Vital: Tool: File //
///////////////////////

namespace Vital::Tool::File {
    inline bool sanitize(const godot::String& path) {
        if (path.is_empty()) return false;
        if (path.begins_with("/") || path.begins_with("\\")) return false;
        if (path.find(":") != -1) return false;
        auto parts = path.replace("\\", "/").split("/");
        for (const auto& part : parts) {
            if (part == "..")
                return false;
        }
        return true;
    }

    inline bool sanitize(const std::string& base) {
        return sanitize(to_godot_string(base));
    }

    inline bool exists(const godot::String& base, const godot::String& target) {
        if (!sanitize(target)) throw Vital::Log::fetch("file-path-invalid", Vital::Log::Type::Error, to_std_string(target));
        auto dir = godot::DirAccess::open(base);
        return dir.is_valid() && dir -> file_exists(target);
    }

    inline bool exists(const std::string& base, const std::string& target) {
        return exists(to_godot_string(base), to_godot_string(target));
    }

    inline uint64_t size(const godot::String& base, const godot::String& target) {
        if (!sanitize(target)) throw Vital::Log::fetch("file-path-invalid", Vital::Log::Type::Error, to_std_string(target));
        auto dir = godot::DirAccess::open(base);
        if (!dir.is_valid()) throw Vital::Log::fetch("base-path-invalid", Vital::Log::Type::Error, to_std_string(base));
        if (!dir -> file_exists(target)) throw Vital::Log::fetch("file-nonexistent", Vital::Log::Type::Error, to_std_string(target));
        auto file = godot::FileAccess::open(dir -> get_current_dir() + "/" + target, godot::FileAccess::READ);
        if (!file.is_valid()) throw Vital::Log::fetch("file-busy", Vital::Log::Type::Error, to_std_string(target));
        return file -> get_length();
    }

    inline uint64_t size(const std::string& base, const std::string& target) {
        return size(to_godot_string(base), to_godot_string(target));
    }

    inline std::string hash(const godot::String& base, const godot::String& target, std::string_view mode = "SHA256") {
        if (!sanitize(target)) throw Vital::Log::fetch("file-path-invalid", Vital::Log::Type::Error, to_std_string(target));
        auto dir = godot::DirAccess::open(base);
        if (!dir.is_valid()) throw Vital::Log::fetch("base-path-invalid", Vital::Log::Type::Error, to_std_string(base));
        if (!dir -> file_exists(target)) throw Vital::Log::fetch("file-nonexistent", Vital::Log::Type::Error, to_std_string(target));
        auto full_path = to_std_string(dir -> get_current_dir() + "/" + target);
        return Vital::Tool::Crypto::hash_file(mode, full_path);
    }

    inline std::string hash(const std::string& base, const std::string& target, std::string_view mode = "SHA256") {
        return hash(to_godot_string(base), to_godot_string(target), mode);
    }

    inline bool remove(const godot::String& base, const godot::String& target) {
        if (!sanitize(target)) throw Vital::Log::fetch("file-path-invalid", Vital::Log::Type::Error, to_std_string(target));
        auto dir = godot::DirAccess::open(base);
        if (!dir.is_valid()) throw Vital::Log::fetch("base-path-invalid", Vital::Log::Type::Error, to_std_string(base));
        if (!dir -> file_exists(target)) return false;
        return dir -> remove(target) == godot::OK;
    }

    inline bool remove(const std::string& base, const std::string& target) {
        return remove(to_godot_string(base), to_godot_string(target));
    }

    inline std::string read_text(const godot::String& base, const godot::String& target) {
        if (!sanitize(target)) throw Vital::Log::fetch("file-path-invalid", Vital::Log::Type::Error, to_std_string(target));
        auto dir = godot::DirAccess::open(base);
        if (!dir.is_valid()) throw Vital::Log::fetch("base-path-invalid", Vital::Log::Type::Error, to_std_string(base));
        if (!dir -> file_exists(target)) throw Vital::Log::fetch("file-nonexistent", Vital::Log::Type::Error, to_std_string(target));
        auto file = godot::FileAccess::open(dir -> get_current_dir() + "/" + target, godot::FileAccess::READ);
        if (!file.is_valid()) throw Vital::Log::fetch("file-busy", Vital::Log::Type::Error, to_std_string(target));
        return to_std_string(file -> get_as_text());
    }

    inline std::string read_text(const std::string& base, const std::string& target) { 
        return read_text(to_godot_string(base), to_godot_string(target));
    }

    inline godot::PackedByteArray read_binary(const godot::String& base, const godot::String& target) {
        if (!sanitize(target)) throw Vital::Log::fetch("file-path-invalid", Vital::Log::Type::Error, to_std_string(target));
        auto dir = godot::DirAccess::open(base);
        if (!dir.is_valid()) throw Vital::Log::fetch("base-path-invalid", Vital::Log::Type::Error, to_std_string(base));
        if (!dir -> file_exists(target)) throw Vital::Log::fetch("file-nonexistent", Vital::Log::Type::Error, to_std_string(target));
        auto file = godot::FileAccess::open(dir -> get_current_dir() + "/" + target, godot::FileAccess::READ);
        if (!file.is_valid()) throw Vital::Log::fetch("file-busy", Vital::Log::Type::Error, to_std_string(target));
        return file -> get_buffer(file -> get_length());
    }

    inline godot::PackedByteArray read_binary(const std::string& base, const std::string& target) {
        return read_binary(to_godot_string(base), to_godot_string(target));
    }

    inline bool write_text(const godot::String& base, const godot::String& target, const std::string& text) {
        if (!sanitize(target)) throw Vital::Log::fetch("file-path-invalid", Vital::Log::Type::Error, to_std_string(target));
        godot::DirAccess::make_dir_recursive_absolute(base);
        auto dir = godot::DirAccess::open(base);
        if (!dir.is_valid()) throw Vital::Log::fetch("base-path-invalid", Vital::Log::Type::Error, to_std_string(base));
        auto path = dir -> get_current_dir() + "/" + target;
        godot::DirAccess::make_dir_recursive_absolute(path.get_base_dir());
        auto file = godot::FileAccess::open(path, godot::FileAccess::WRITE);
        if (!file.is_valid()) throw Vital::Log::fetch("file-busy", Vital::Log::Type::Error, to_std_string(target));
        file -> store_string(to_godot_string(text));
        return true;
    }

    inline bool write_text(const std::string& base, const std::string& target, const std::string& text) {
        return write_text(to_godot_string(base), to_godot_string(target), text);
    }

    inline bool write_binary(const godot::String& base, const godot::String& target, const godot::PackedByteArray& data) {
        if (!sanitize(target)) throw Vital::Log::fetch("file-path-invalid", Vital::Log::Type::Error, to_std_string(target));
        godot::DirAccess::make_dir_recursive_absolute(base);
        auto dir = godot::DirAccess::open(base);
        if (!dir.is_valid()) throw Vital::Log::fetch("base-path-invalid", Vital::Log::Type::Error, to_std_string(base));
        auto path = dir -> get_current_dir() + "/" + target;
        godot::DirAccess::make_dir_recursive_absolute(path.get_base_dir());
        auto file = godot::FileAccess::open(path, godot::FileAccess::WRITE);
        if (!file.is_valid()) throw Vital::Log::fetch("file-busy", Vital::Log::Type::Error, to_std_string(target));
        file -> store_buffer(data);
        return true;
    }

    inline bool write_binary(const std::string& base, const std::string& target, const godot::PackedByteArray& data) {
        return write_binary(to_godot_string(base), to_godot_string(target), data);
    }

    inline std::vector<std::string> contents(const godot::String& base, const godot::String& target, bool directory_search = false) {
        if (!sanitize(target)) throw Vital::Log::fetch("file-path-invalid", Vital::Log::Type::Error, to_std_string(target));
        auto dir = godot::DirAccess::open(base);
        if (!dir.is_valid()) throw Vital::Log::fetch("base-path-invalid", Vital::Log::Type::Error, to_std_string(base));
        dir = godot::DirAccess::open(dir -> get_current_dir() + "/" + target);
        if (!dir.is_valid()) throw Vital::Log::fetch("directory-nonexistent", Vital::Log::Type::Error, to_std_string(target));
        std::vector<std::string> result;
        dir -> list_dir_begin();
        while (true) {
            auto name = dir -> get_next();
            if (name.is_empty()) break;
            if (name == "." || name == "..") continue;
            if (directory_search != dir -> current_is_dir()) continue;
            result.emplace_back(to_std_string(target.is_empty() ? name : target + godot::String("/") + name));
        }
        dir -> list_dir_end();
        return result;
    }

    inline std::vector<std::string> contents(const std::string& base, const std::string& target, bool directory_search = false) {
        return contents(to_godot_string(base), to_godot_string(target), directory_search);
    }

    inline std::vector<std::string> glob(const godot::String& base, const godot::String& pattern) {
        std::vector<std::string> result;
        int last_slash = std::max(pattern.rfind("/"), pattern.rfind("\\"));
        auto dir_part = last_slash >= 0 ? pattern.substr(0, last_slash) : godot::String();
        auto file_part = last_slash >= 0 ? pattern.substr(last_slash + 1) : pattern;
        bool has_recursive_wildcard = dir_part.find("**") != -1;
        bool has_single_wildcard = file_part.find("*") != -1;

        if (!has_recursive_wildcard && !has_single_wildcard) {
            auto full_path = dir_part.is_empty() ? file_part : (dir_part + godot::String("/") + file_part);
            if (exists(base, full_path)) result.emplace_back(to_std_string(full_path));
            return result;
        }
    
        auto matches_file = [&](const godot::String& name) {
            if (contains_wildcard(to_std_string(file_part))) return match_wildcard(to_std_string(file_part), to_std_string(name));
            return name == file_part;
        };
    
        if (has_recursive_wildcard) {
            auto search_base = dir_part.replace("**", godot::String());
            if (search_base.ends_with("/")) search_base = search_base.substr(0, search_base.length() - 1);
            std::function<void(const godot::String&)> walk = [&](const godot::String& current_dir) {
                auto full_dir  = search_base.is_empty() ? current_dir : (search_base + godot::String("/") + current_dir);
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
                    else if (matches_file(name)) result.emplace_back(to_std_string(full_path));
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
                        result.emplace_back(to_std_string(full_path));
                    }
                }
                dir -> list_dir_end();
            }
        }
        return result;
    }

    inline std::vector<std::string> glob(const std::string& base, const std::string& pattern) {
        return glob(to_godot_string(base), to_godot_string(pattern));
    }
}