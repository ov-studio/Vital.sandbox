/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: index.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Root Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <pch.h>
#include <godot_cpp/classes/os.hpp>
#include <Vital.sandbox/Tool/rest.h>
#include <Vital.sandbox/Vendor/rapidjson/document.h>


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

    inline unsigned int get_tick() {
        return static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()/1000000);
    }

    inline std::string get_directory() {
        return to_std_string(godot::OS::get_singleton() -> get_executable_path().get_base_dir());
    }
    
    inline std::vector<std::string> get_modules(const std::string& name) {
        std::vector<std::string> result;
        rapidjson::Document manifest;
        manifest.Parse(Vital::System::Rest::get(fmt::format(Repository, "manifest.json")).c_str());
        if (!manifest.HasParseError() && manifest.HasMember(name.c_str())) {
            for (auto& i : manifest[name.c_str()]["sources"].GetArray()) {
                result.push_back(Vital::System::Rest::get(fmt::format(Repository, name + "/" + std::string(i.GetString()))));
            }
        }
        return result;
    }
}