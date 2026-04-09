/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: font.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Font Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Engine/public/index.h>


//////////////////////////
// Vital: Engine: Font //
//////////////////////////

namespace Vital::Engine {
    class Font : public godot::Node2D {
        private:
            godot::Ref<godot::FontFile> font;
        public:
            // Instantiators //
            Font(const godot::Ref<godot::FontFile>& font);
            ~Font() override = default;


            // Managers //
            static Font* create(const std::string& path);
            static Font* create_from_buffer(const godot::PackedByteArray& buffer);
            void destroy();


            // Setters //
            void set_antialiasing(bool enabled);
            void set_oversampling(float oversampling);


            // Getters //
            godot::Ref<godot::FontFile> get_font();
            bool get_antialiasing();
            float get_oversampling();
    };
}
#endif