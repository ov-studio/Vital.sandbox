/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: image.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Image Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/texture.h>


///////////////////////////
// Vital: Engine: Image //
///////////////////////////

namespace Vital::Engine {
    class Image : public Texture {
        public:
            enum class Format {
                JPG,
                PNG,
                WEBP,
                BMP,
                DDS,
                KTX,
                EXR,
                UNKNOWN
            };

            inline static const std::vector<Tool::Format::Descriptor<Format>> format_registry = {
                { Format::JPG,  "jpg",  { 0xFF, 0xD8 }                                     },
                { Format::PNG,  "png",  { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A } },
                { Format::WEBP, "webp", { 0x52, 0x49, 0x46, 0x46 }                         },
                { Format::BMP,  "bmp",  { 0x42, 0x4D }                                     },
                { Format::DDS,  "dds",  { 0x44, 0x44, 0x53, 0x20 }                         },
                { Format::KTX,  "ktx",  { 0xAB, 0x4B, 0x54, 0x58, 0x20 }                   },
                { Format::EXR,  "exr",  { 0x76, 0x2F, 0x31, 0x01 }                         }
            };
        protected:
            godot::Ref<godot::ImageTexture> texture;


            // Instantiators //
            Image(const godot::Ref<godot::ImageTexture>& texture, const std::string& reference = "");
        public:
            // Checkers //
            bool is_compressed() const override;


            // Getters //
            godot::Ref<godot::ImageTexture> get_texture() const override { return texture; }


            // Misc //
            static Image* create_texture_2d(const std::string& base, const std::string& path, bool mipmaps = false, const std::string& reference = "");
            static Image* create_texture_2d_from_buffer(const godot::PackedByteArray& buffer, bool mipmaps = false, const std::string& reference = "");
            void convert(godot::Image::Format format);
            void compress(godot::Image::CompressMode mode);
    };
}
#endif
