/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: svg.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SVG Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/texture.h>


/////////////////////////
// Vital: Engine: SVG //
/////////////////////////

namespace Vital::Engine {
    class SVG : public Texture {
        protected:
            godot::Ref<godot::ImageTexture> texture;


            // Instantiators //
            SVG(const godot::Ref<godot::ImageTexture>& texture, const std::string& reference = "");
        public:
            // Getters //
            godot::Ref<godot::ImageTexture> get_texture() const override { return texture; }


            // Misc //
            static SVG* create_svg(const std::string& base, const std::string& path, bool mipmaps = false, const std::string& reference = "");
            static SVG* create_svg_from_raw(const std::string& raw, bool mipmaps = false, const std::string& reference = "");
            static SVG* create_svg_from_buffer(const godot::PackedByteArray& buffer, bool mipmaps = false, const std::string& reference = "");
            void update_svg_from_raw(const std::string& raw);
            void update_svg_from_buffer(const godot::PackedByteArray& buffer);
    };
}
#endif
