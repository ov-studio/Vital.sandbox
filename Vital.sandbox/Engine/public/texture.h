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
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


/////////////////////////////
// Vital: Engine: Texture //
/////////////////////////////

namespace Vital::Engine {
    class Texture : public godot::Node2D {
        public:
            enum class Type {
                Texture2D,
                SVG
            };

            enum class Format {
                PNG,
                JPG,
                WEBP,
                UNKNOWN
            };

            struct Texture2D {
                godot::Ref<godot::Texture2D> texture;
            };

            struct SVG {
                godot::Ref<godot::ImageTexture> texture;
            };
        
            struct Command {
                Type type;
                std::variant<Texture2D, SVG> payload;
            };

            inline static const unsigned int flush_interval = 10000;
        protected:
            Command command;
            uint64_t reference_tick = 0;
            std::string reference_key = "";
            inline static std::unordered_map<std::string, Texture*> reference_cache = {};
        public:
            // Instantiators //
            Texture(Command cmd, const std::string& reference = "");
            ~Texture();


            // Managers //
            void destroy();
            void heartbeat();
            static void flush();


            // Getters //
            static Format get_format(const godot::PackedByteArray& buffer);
            static Texture* get_from_reference(const std::string& reference);
            godot::Ref<godot::Texture2D> get_texture() const;


            // APIs //
            static Texture* create_texture_2d(const std::string& path, const std::string& reference = "");
            static Texture* create_texture_2d_from_buffer(const godot::PackedByteArray& buffer, const std::string& reference = "");
            static Texture* create_svg(const std::string& path, const std::string& reference = "");
            static Texture* create_svg_from_raw(const std::string& raw, const std::string& reference = "");
            static Texture* create_svg_from_buffer(const godot::PackedByteArray& buffer, const std::string& reference = "");
            void update_svg_from_raw(const std::string& raw);
            void update_svg_from_buffer(const godot::PackedByteArray& buffer);
    };
}
#endif