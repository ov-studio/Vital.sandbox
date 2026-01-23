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
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/engine.hpp>


////////////
// Vital //
////////////

namespace Vital {
    namespace Util {}
    namespace System {}
    namespace Sandbox {}
    static const std::string Signature = "vsdk_v.0.0.1";
    static const std::string Repository = "https://raw.githubusercontent.com/ov-studio/Vital.sandbox/refs/heads/module/{}";

    inline godot::String to_godot_string(const std::string& input) {
        return godot::String::utf8(input.c_str());
    }

    inline std::string to_std_string(const godot::String& input) {
        return std::string(input.utf8().get_data());
    }

    inline const std::string get_platform() { 
        #if defined(Vital_SDK_Client)
            return "client";
        #else
            return "server";
        #endif
    }

    inline int64_t get_tick() {
        auto now = std::chrono::steady_clock::now();
        auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        return static_cast<int64_t>(ms);
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