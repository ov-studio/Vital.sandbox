/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: shader.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 22/08/2026
     Desc: Shader Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


///////////////////////////
// Vital: Engine: Shader //
///////////////////////////

// TODO: Improve

namespace Vital::Engine {
    class Shader {
        public:
            // Shader types mirroring GDShader's shader_type keyword.
            // CANVAS_ITEM is the type used for 2-D canvas / postprocess draws
            // via Canvas::draw_shader.  SPATIAL is for 3-D mesh surface overrides.
            enum class Type {
                Spatial,
                CanvasItem
            };

        private:
            godot::Ref<godot::Shader>         gd_shader;
            godot::Ref<godot::ShaderMaterial> gd_material;
            Type shader_type = Type::CanvasItem;

            // Instantiators //
            Shader() = default;
            ~Shader() = default;

        public:
            // Managers //
            static Shader* create(const std::string& code, Type type = Type::CanvasItem);
            void destroy();

            // Checkers //
            bool is_valid() const;

            // Getters //
            Type get_type() const;
            std::string get_code() const;
            godot::Ref<godot::ShaderMaterial> get_material() const;

            // Setters //
            bool set_code(const std::string& code);
            bool set_param(const std::string& name, const godot::Variant& value);
            bool set_param_texture(const std::string& name, godot::Ref<godot::Texture2D> texture);
            bool set_param_viewport_texture(const std::string& name, godot::Ref<godot::ViewportTexture> texture);

            // Misc //
            // Apply this shader material to every surface of a MeshInstance3D node
            // and all MeshInstance3D children (recursive). Returns surfaces patched.
            // Only meaningful for Type::Spatial shaders.
            int apply_to_node(godot::Node* node);
    };
}
#endif
