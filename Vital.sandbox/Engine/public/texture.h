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
            enum class Type {
                Texture2D,
                SVG
            };

            enum class Format {
                PNG,
                JPG,
                WEBP,
                BMP,
                DDS,
                KTX,
                UNKNOWN
            };

            inline static const std::vector<Tool::Format::Descriptor<Format>> format_registry = {
                { Format::JPG, "jpg", { 0xFF, 0xD8 } },
                { Format::PNG, "png", { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A } },
                { Format::WEBP, "webp", { 0x52, 0x49, 0x46, 0x46 } },
                { Format::BMP, "bmp", { 0x42, 0x4D } },
                { Format::DDS, "dds", { 0x44, 0x44, 0x53, 0x20 } },
                { Format::KTX, "ktx", { 0xAB, 0x4B, 0x54, 0x58, 0x20 } }
            }

            inline static const std::unordered_set<godot::Image::Format> convert_registry = {
                godot::Image::FORMAT_L8,
                godot::Image::FORMAT_LA8,
                godot::Image::FORMAT_RGB8,
                godot::Image::FORMAT_RGBA8,
                godot::Image::FORMAT_RGBA4444,
                godot::Image::FORMAT_RGB565
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
            static Texture* get_from_reference(const std::string& reference);
            godot::Ref<godot::Texture2D> get_texture() const;
            godot::Vector2i get_size() const;


            // APIs //
            static Texture* create_texture_2d(const std::string& path, const std::string& reference = "");
            static Texture* create_texture_2d_from_buffer(const godot::PackedByteArray& buffer, const std::string& reference = "");
            static Texture* create_svg(const std::string& path, const std::string& reference = "");
            static Texture* create_svg_from_raw(const std::string& raw, const std::string& reference = "");
            static Texture* create_svg_from_buffer(const godot::PackedByteArray& buffer, const std::string& reference = "");
            void update_svg_from_raw(const std::string& raw);
            void update_svg_from_buffer(const godot::PackedByteArray& buffer);
            void convert(godot::Image::Format format);
    };
}
#endif