/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: texture.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Texture Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

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
        protected:
            Command command;
        public:
            // Instantiators //
            Texture() = default;
            ~Texture() override = default;


            // Getters //
            godot::Ref<godot::Texture2D> get_texture();


            // APIs //
            static Texture* create_texture_2d(const std::string& path);
            static Texture* create_svg(const std::string& path);
    };
}