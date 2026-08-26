/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: texture.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Texture Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


/////////////////////////////
// Vital: Engine: Texture //
/////////////////////////////

namespace Vital::Engine {
    class Texture : public godot::Node2D {
        public:
            inline static const unsigned int flush_interval = 10000;
        protected:
            uint64_t reference_tick = 0;
            std::string reference_key = "";
            godot::Ref<godot::CanvasTexture> canvas_texture;
            inline static std::unordered_map<std::string, Texture*> reference_cache = {};


            // Instantiators //
            Texture(const std::string& reference = "");
            virtual ~Texture();
        public:
            // Managers //
            void destroy();
            void heartbeat();
            static void flush();


            // Checkers //
            bool has_mipmaps() const;
            virtual bool is_compressed() const { return false; }


            // Getters //
            static Texture* get_from_reference(const std::string& reference);
            virtual godot::Ref<godot::ImageTexture> get_texture() const = 0;
            godot::Ref<godot::Texture2D> get_canvas_texture() const;
            godot::Vector2i get_size() const;
            godot::CanvasItem::TextureFilter get_filter() const;


            // Setters //
            void set_filter(godot::CanvasItem::TextureFilter mode);
    };
}
#endif
