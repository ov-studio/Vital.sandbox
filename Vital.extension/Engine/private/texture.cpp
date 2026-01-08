/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: texture.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Texture Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#include <Vital.extension/Engine/public/texture.h>


////////////////////////////
// Vital: Godot: Texture //
////////////////////////////

namespace Vital::Godot {
    // Getters //
    godot::Ref<godot::Texture2D> Texture::get_texture() {
        switch (command.type) {
            case Type::Texture2D: {
                const auto& payload = std::get<Texture2D>(command.payload);
                return payload.texture;
            }
            case Type::SVG: {
                const auto& payload = std::get<SVG>(command.payload);
                return payload.texture;
            }
        }
        return godot::Ref<godot::Texture2D>();
    }


    // APIs //
    Texture* Texture::create_texture_2d(const std::string& path) {
        auto* texture = memnew(Texture);
        return texture;
    }

    Texture* Texture::create_svg(const std::string& path) {
        auto buffer = godot::FileAccess::get_file_as_string(path.c_str());
        if (buffer.is_empty()) throw std::runtime_error(ErrorCode["invalid-arguments"]);
        SVG payload;
        auto* texture = memnew(Texture);
        godot::Ref<godot::Image> image;
        image.instantiate();
        auto status = image -> load_svg_from_string(buffer, 1.0);
        if (status != godot::OK) {
            memdelete(texture);
            throw std::runtime_error(ErrorCode["invalid-arguments"]);
        }
        payload.texture = godot::ImageTexture::create_from_image(image);
        texture -> command = {Type::SVG, payload};
        return texture;
    }
}