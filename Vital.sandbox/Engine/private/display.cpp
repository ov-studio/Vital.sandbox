/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: display.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Display Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/display.h>


/////////////////////////////
// Vital: Engine: Display //
/////////////////////////////

namespace Vital::Engine {
    // Getters //
    godot::Vector2 Display::get_resolution() {
        return Core::get_display_server() -> window_get_size();
    }

    godot::Ref<godot::ViewportTexture> Display::get_texture() {
        return Core::get_scene_root() -> get_texture();
    }


    // Misc //
    void Display::screenshot(const std::string& base, const std::string& path) {
        auto target = Tool::to_godot_string(base + "/" + path);
        auto image = get_texture() -> get_image();
        godot::DirAccess::make_dir_recursive_absolute(target.get_base_dir());
        if (!image.is_valid()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "failed to capture screenshot");
        if (image -> save_png(target) != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("failed to save screenshot"));
    }
}
#endif
