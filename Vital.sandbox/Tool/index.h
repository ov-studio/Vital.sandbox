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
#include <Vital.sandbox/Tool/stack.h>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/engine.hpp>


////////////
// Vital //
////////////

namespace Vital {
    namespace System {}
    static const std::string Build_ver = "v0.0.1";
    static const std::string Repo_Kit = "https://raw.githubusercontent.com/ov-studio/Vital.kit/refs/heads/main/{}";

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

    inline std::string get_directory() {
        return to_std_string(godot::OS::get_singleton() -> get_executable_path().get_base_dir());
    }

    inline bool is_editor() { 
        #if defined(Vital_SDK_Client)
            return godot::Engine::get_singleton() -> is_editor_hint();
        #else
            return false;
        #endif
    }
}