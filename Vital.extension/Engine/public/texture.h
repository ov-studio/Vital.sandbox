/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: texture.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Texture Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#if defined(Vital_SDK_Client)
#pragma once
#include <Vital.extension/Engine/public/index.h>


////////////////////////////
// Vital: Godot: Texture //
////////////////////////////

namespace Vital::Godot {
    class Texture;
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
                unsigned int tick;
                std::variant<Texture2D, SVG> payload;
            };
        protected:
            Command command;
            inline static const unsigned int flush_interval = 10000;
            inline static std::unordered_map<std::string, Texture*> cache_temp = {};
            void push_temp(const std::string& temp_ref);
        public:
            // Instantiators //
            Texture() = default;
            ~Texture() override = default;
            void heartbeat();
            static void flush();


            // Getters //
            static Format get_format(const godot::PackedByteArray& buffer);
            static Texture* get_from_ref(const std::string& temp_ref);
            godot::Ref<godot::Texture2D> get_texture();


            // APIs //
            static Texture* create_texture_2d(const std::string& path, const std::string& temp_ref = "");
            static Texture* create_texture_2d_from_buffer(const godot::PackedByteArray& buffer, const std::string& temp_ref = "");
            static Texture* create_svg(const std::string& path, const std::string& temp_ref = "");
            static Texture* create_svg_from_raw(const std::string& raw, const std::string& temp_ref = "");
            static Texture* create_svg_from_buffer(const godot::PackedByteArray& buffer, const std::string& temp_ref = "");
            void update_svg_from_raw(const std::string& raw);
            void update_svg_from_buffer(const godot::PackedByteArray& buffer);
    };
}
#endif