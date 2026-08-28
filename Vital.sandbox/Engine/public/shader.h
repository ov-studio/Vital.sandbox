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


////////////////////////////
// Vital: Engine: Shader //
////////////////////////////

namespace Vital::Engine {
    class Shader {
        public:
            enum class Mode {
                Spatial,
                CanvasItem
            };
        private:
            godot::Ref<godot::Shader> shader;
            godot::Ref<godot::ShaderMaterial> material;
            Mode shader_type = Mode::CanvasItem;

            struct Internal {
                static constexpr const char* SENTINEL = "vsdk_sentinel";

                struct EntryPoint {
                    std::string signature;
                    std::string usage;
                };


                // Helpers //
                static std::string inject_sentinel(const std::string& src, bool is_spatial);
                static bool validate_compiled(godot::Ref<godot::Shader>& shader);
                static std::string build_source(const std::string& raw, Mode mode);
            };


            // Instantiators //
            Shader() = default;
            ~Shader() = default;
        public:
            // Managers //
            static Shader* create(const std::string& base, const std::string& path, Mode mode = Mode::CanvasItem);
            static Shader* create_from_raw(const std::string& raw, Mode mode = Mode::CanvasItem);
            void destroy();


            // Checkers //
            bool is_valid() const;


            // Getters //
            Mode get_type() const;
            std::string get_code() const;
            godot::Ref<godot::ShaderMaterial> get_material() const;


            // Setters //
            bool set_param(const std::string& name, const godot::Variant& value);
            bool set_param_texture(const std::string& name, godot::Ref<godot::Texture2D> texture);
            bool set_param_viewport_texture(const std::string& name, godot::Ref<godot::ViewportTexture> texture);


            // Misc //
            int apply_to_node(godot::Node* node);
    };
}
#endif
