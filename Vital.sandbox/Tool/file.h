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
#include <Vital.sandbox/Tool/error.h>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>


////////////////////////
// Vital: Tool: File //
///////////////////////

namespace Vital::Tool::File {
    inline bool is_path(const godot::String& path) {
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

    inline bool exists(const godot::String& base, const godot::String& target) {
        if (!is_path(target)) throw Vital::Error::fetch("file-path-invalid", to_std_string(target));
        auto dir = godot::DirAccess::open(base);
        if (!dir.is_valid()) throw Vital::Error::fetch("base-path-invalid", to_std_string(base));
        return dir -> file_exists(target);
    }

    inline uint64_t size(const godot::String& base, const godot::String& target) {
        if (!is_path(target)) throw Vital::Error::fetch("file-path-invalid", to_std_string(target));
        auto dir = godot::DirAccess::open(base);
        if (!dir.is_valid()) throw Vital::Error::fetch("base-path-invalid", to_std_string(base));
        if (!dir -> file_exists(target)) throw Vital::Error::fetch("file-nonexistent", to_std_string(target));
        auto file = godot::FileAccess::open(dir -> get_current_dir() + "/" + target, godot::FileAccess::READ);
        if (!file.is_valid()) throw Vital::Error::fetch("file-busy", to_std_string(target));
        return file -> get_length();
    }

    inline bool remove(const godot::String& base, const godot::String& target) {
        if (!is_path(target)) throw Vital::Error::fetch("file-path-invalid", to_std_string(target));
        auto dir = godot::DirAccess::open(base);
        if (!dir.is_valid()) throw Vital::Error::fetch("base-path-invalid", to_std_string(base));
        if (!dir -> file_exists(target)) return false;
        return dir -> remove(target) == godot::OK;
    }

    inline std::string read_text(const godot::String& base, const godot::String& target) {
        if (!is_path(target)) throw Vital::Error::fetch("file-path-invalid", to_std_string(target));
        auto dir = godot::DirAccess::open(base);
        if (!dir.is_valid()) throw Vital::Error::fetch("base-path-invalid", to_std_string(base));
        if (!dir -> file_exists(target)) throw Vital::Error::fetch("file-nonexistent", to_std_string(target));
        auto file = godot::FileAccess::open(dir -> get_current_dir() + "/" + target, godot::FileAccess::READ);
        if (!file.is_valid()) throw Vital::Error::fetch("file-busy", to_std_string(target));
        return to_std_string(file -> get_as_text());
    }

    inline godot::PackedByteArray read_binary(const godot::String& base, const godot::String& target) {
        if (!is_path(target)) throw Vital::Error::fetch("file-path-invalid", to_std_string(target));
        auto dir = godot::DirAccess::open(base);
        if (!dir.is_valid()) throw Vital::Error::fetch("base-path-invalid", to_std_string(base));
        if (!dir -> file_exists(target)) throw Vital::Error::fetch("file-nonexistent", to_std_string(target));
        auto file = godot::FileAccess::open(dir -> get_current_dir() + "/" + target, godot::FileAccess::READ);
        if (!file.is_valid()) throw Vital::Error::fetch("file-busy", to_std_string(target));
        return file -> get_buffer(file -> get_length());
    }

    inline bool write_text(const godot::String& base, const godot::String& target, const std::string& text) {
        if (!is_path(target)) throw Vital::Error::fetch("file-path-invalid", to_std_string(target));
        auto dir = godot::DirAccess::open(base);
        if (!dir.is_valid()) throw Vital::Error::fetch("base-path-invalid", to_std_string(base));
        auto file = godot::FileAccess::open(dir -> get_current_dir() + "/" + target, godot::FileAccess::WRITE);
        if (!file.is_valid()) throw Vital::Error::fetch("file-busy", to_std_string(target));
        file -> store_string(to_godot_string(text));
        return true;
    }

    inline bool write_binary(const godot::String& base, const godot::String& target, const godot::PackedByteArray& data) {
        if (!is_path(target)) throw Vital::Error::fetch("file-path-invalid", to_std_string(target));
        auto dir = godot::DirAccess::open(base);
        if (!dir.is_valid()) throw Vital::Error::fetch("base-path-invalid", to_std_string(base));
        auto file = godot::FileAccess::open(dir -> get_current_dir() + "/" + target, godot::FileAccess::WRITE);
        if (!file.is_valid()) throw Vital::Error::fetch("file-busy", to_std_string(target));
        file -> store_buffer(data);
        return true;
    }

    inline std::vector<std::string> contents(const godot::String& base, const godot::String& target, bool directory_search = false) {
        if (!is_path(target)) throw Vital::Error::fetch("file-path-invalid", to_std_string(target));
        auto dir = godot::DirAccess::open(base);
        if (!dir.is_valid()) throw Vital::Error::fetch("base-path-invalid", to_std_string(base));
        dir = godot::DirAccess::open(dir -> get_current_dir() + "/" + target);
        if (!dir.is_valid()) throw Vital::Error::fetch("directory-nonexistent", to_std_string(target));
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
}