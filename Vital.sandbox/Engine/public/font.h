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
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


//////////////////////////
// Vital: Engine: Font //
//////////////////////////

namespace Vital::Engine {
    class Font : public godot::Node2D {
        GDCLASS(Font, godot::Node2D)
        private:
            godot::Ref<godot::FontFile> font;


            // Instantiators //
            Font(const godot::Ref<godot::FontFile>& font);
            ~Font() override = default;
        public:
            // Managers //
            static Font* create(const std::string& base, const std::string& path);
            static Font* create_from_buffer(const godot::PackedByteArray& buffer);
            void destroy();


            // Checkers //
            bool is_antialiased() const;

            
            // Getters //
            godot::Ref<godot::FontFile> get_font() const;
            float get_oversampling() const;


            // Setters //
            void set_antialiased(bool state);
            void set_oversampling(float oversampling);
    };
}
#endif