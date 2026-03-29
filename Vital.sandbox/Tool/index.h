/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: index.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vital Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <pch.h>
#include <Vital.sandbox/Tool/version.h>
#include <Vital.sandbox/Tool/stack.h>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/engine.hpp>


////////////
// Vital //
////////////

namespace Vital {
    namespace System {}

    static std::string indent(int level) {
        #if defined(Vital_SDK_Client)
        return std::string(level*4, ' ');
        #else
        return std::string(level*2, ' ');
        #endif
    }

    inline godot::String to_godot_string(const std::string& input) {
        return godot::String::utf8(input.c_str());
    }

    inline std::string to_std_string(const godot::String& input) {
        return std::string(input.utf8().get_data());
    }

    inline uint64_t get_tick() {
        return godot::Time::get_singleton() -> get_ticks_msec();
    }

    inline const std::string get_platform() { 
        #if defined(Vital_SDK_Client)
            return "client";
        #else
            return "server";
        #endif
    }

    inline Vital::Tool::Stack get_timestamp() {
        Vital::Tool::Stack timestamp;
        godot::Dictionary datetime = godot::Time::get_singleton() -> get_datetime_dict_from_system();
        timestamp.object["hour"] = int(datetime["hour"]);
        timestamp.object["minute"] = int(datetime["minute"]);
        timestamp.object["second"] = int(datetime["second"]);
        timestamp.object["day"] = int(datetime["day"]);
        timestamp.object["month"] = int(datetime["month"]);
        timestamp.object["year"] = int(datetime["year"]);
        return timestamp;
    }

    template<typename... Args>
    inline std::string get_directory(Args&&... args) {
        std::string base = to_std_string(godot::OS::get_singleton() -> get_executable_path().get_base_dir());
        ((base += "/" + std::string(std::forward<Args>(args))), ...);
        return base;
    }

    inline bool is_runtime() { 
        if (godot::Engine::get_singleton() -> is_editor_hint()) return false;
        godot::PackedStringArray args = godot::OS::get_singleton() -> get_cmdline_args();
        for (int i = 0; i < args.size(); i++) {
            if (args[i] == "--headless" || args[i] == "--export-release" || args[i] == "--export-debug") return false;
        }
        return true;
    }

    inline bool contains_wildcard(const std::string& input) {
        return input.find('*') != std::string::npos;
    }

    inline bool match_wildcard(const std::string& pattern, const std::string& input) {
        const char* p = pattern.c_str();
        const char* n = input.c_str();
        const char* star = nullptr;
        const char* match = n;
        while (*n) {
            if (*p == '*') { star = p++; match = n; }
            else if (*p == *n) { p++; n++; }
            else if (star) { p = star + 1; n = ++match; }
            else return false;
        }
        while (*p == '*') p++;
        return *p == '\0';
    }
}