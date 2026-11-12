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

// TODO: Improve
namespace Vital::Engine {
    class Rendertarget;
    class Shader {
        public:
            enum class Mode {
                Spatial,
                CanvasItem,
                Postprocess
            };

            inline static const std::vector<std::pair<std::string, Mode>> mode_registry = {
                { "CANVAS_ITEM",   Mode::CanvasItem   },
                { "SPATIAL",       Mode::Spatial      },
                { "POST_PROCESS",  Mode::Postprocess  }
            };
        private:
            inline static std::vector<Shader*> postprocess_registry;

            godot::Ref<godot::Shader> shader;
            godot::Ref<godot::ShaderMaterial> material;
            Mode mode = Mode::CanvasItem;
            godot::ObjectID pp_quad_id;
            godot::Ref<godot::ShaderMaterial> pp_material;
            std::vector<godot::Ref<godot::ShaderMaterial>> surface_materials;
            uint64_t pp_last_frame = UINT64_MAX;
            
            godot::MeshInstance3D* pp_get_quad() const;

            void postprocess_init();
            void postprocess_free();
            void prune_surface_materials();

            struct Draw_Clone_Pool {
                std::vector<godot::Ref<godot::ShaderMaterial>> clones;
                size_t used = 0;
                uint64_t last_frame = UINT64_MAX;
                godot::Ref<godot::ShaderMaterial> next(godot::Ref<godot::Shader> shader, godot::Ref<godot::ShaderMaterial> source);
                void clear() { clones.clear(); used = 0; last_frame = UINT64_MAX; }
            };
            Draw_Clone_Pool draw_pool;

            struct Output {
                std::string name;
                godot::Ref<godot::Shader> shader;
                godot::ObjectID rendertarget_id;
                Draw_Clone_Pool pool;
            };
            std::vector<Output> outputs;
            godot::Ref<godot::ShaderMaterial> build_surface_clone(godot::Ref<godot::Shader> source, godot::Ref<godot::Material> original);

            struct Internal {
                struct EntryPoint {
                    std::string signature;
                    std::string usage;
                };

                static constexpr const char* SENTINEL = "vsdk_sentinel";

                
                // Helpers //
                static std::string inject_sentinel(const std::string& src, Mode mode);
                static bool validate_compiled(godot::Ref<godot::Shader>& shader);
                static std::string build_source(const std::string& raw, Mode mode, const std::string& active_output = "");
                static std::vector<std::string> scan_outputs(const std::string& raw);
                static std::string rewrite_outputs(const std::string& src, Mode mode, const std::string& active_output);
                static std::string apply_output_render_modes(const std::string& src);
            };


            // Instantiators //
            Shader() = default;
            ~Shader() = default;
        public:
            // Managers //
            static Shader* create(const std::string& base, const std::string& path, Mode mode = Mode::CanvasItem);
            static Shader* create_from_raw(const std::string& raw, Mode mode = Mode::CanvasItem);
            void destroy();
            void draw_postprocess();
            static void end_frame_postprocess();


            // Getters //
            Mode get_mode() const;
            std::string get_code() const;
            godot::Ref<godot::ShaderMaterial> get_material() const;
            godot::Ref<godot::ShaderMaterial> create_surface_material(godot::Ref<godot::Material> original);
            godot::Ref<godot::ShaderMaterial> snapshot_draw_material();


            // Outputs (emulated MRT) //
            size_t get_output_count() const;
            std::vector<std::string> get_output_names() const;
            bool has_output(const std::string& name) const;
            Rendertarget* get_output_target(size_t index) const;
            godot::Ref<godot::ShaderMaterial> snapshot_output_material(size_t index);
            godot::Ref<godot::ShaderMaterial> create_output_surface_material(size_t index, godot::Ref<godot::Material> original);
            bool set_output(const std::string& name, Rendertarget* rendertarget);


            // Setters //
            bool set_param(const std::string& name, const godot::Variant& value);
            bool set_param_rid(const std::string& name, godot::RID texture);
            bool set_param_texture(const std::string& name, godot::Ref<godot::Texture2D> texture);
            bool set_param_viewport_texture(const std::string& name, godot::Ref<godot::ViewportTexture> texture);
    };
}
#endif