/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: font.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Font Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#if defined(Vital_SDK_Client)
#pragma once
#include <Vital.extension/Engine/public/font.h>


//////////////////////////
// Vital: Engine: Font //
//////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Font::Font(const std::string& path) {
        font.instantiate();
        font -> load_dynamic_font(to_godot_string(get_directory()) + to_godot_string("/") + to_godot_string("fonts/Roboto-Bold.ttf"));
    }


    // Managers //
    Font* Font::create(const std::string& path) {
        auto font = memnew(Font(path));
        return font;
    }

    void Font::destroy() {
        memdelete(this);
    }


    // Getters //
    godot::Ref<godot::FontFile> Font::get_font() {
        return font;
    }
}
#endif