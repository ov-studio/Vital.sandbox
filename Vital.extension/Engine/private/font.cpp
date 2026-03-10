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
    Font::Font(const godot::Ref<godot::FontFile>& font) {
        this -> font = font;
    }


    // Managers //
    Font* Font::create(const std::string& path) {
        return create_from_buffer(Vital::Tool::File::read_binary(get_directory(), path));
    }

    Font* Font::create_from_buffer(const godot::PackedByteArray& buffer) {
        godot::Ref<godot::FontFile> font;
        font.instantiate();
        font -> set_data(buffer);
        if (font -> get_data().is_empty()) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
        return memnew(Font(font));
    }

    void Font::destroy() {
        memdelete(this);
    }


    // Setters //
    void Font::set_antialiasing(bool enabled) {
        font -> set_antialiasing(enabled ? godot::TextServer::FONT_ANTIALIASING_GRAY : godot::TextServer::FONT_ANTIALIASING_NONE);
    }

    void Font::set_oversampling(float oversampling) {
        font -> set_oversampling(oversampling);
    }


    // Getters //
    godot::Ref<godot::FontFile> Font::get_font() {
        return font;
    }

    bool Font::get_antialiasing() {
        return font -> get_antialiasing() != godot::TextServer::FONT_ANTIALIASING_NONE;
    }

    float Font::get_oversampling() {
        return font -> get_oversampling();
    }
}
#endif