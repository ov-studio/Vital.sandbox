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
#include <functional>
#include <vector>


////////////////////////////
// Vital: Engine: Shader //
////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    class Shader {
        public:
            enum class Mode {
                Spatial,
                CanvasItem,
                Postprocess  // full-screen quad on camera — gives DEPTH_TEXTURE access
            };

            using SurfaceFactory = std::function<godot::Ref<godot::ShaderMaterial>(godot::Ref<godot::Material>)>;

            inline static const std::vector<std::pair<std::string, Mode>> mode_registry = {
                { "CANVAS_ITEM",  Mode::CanvasItem   },
                { "SPATIAL",      Mode::Spatial      },
                { "POSTPROCESS",  Mode::Postprocess  }
            };
        private:
            godot::Ref<godot::Shader> shader;
            godot::Ref<godot::ShaderMaterial> material;
            Mode mode = Mode::CanvasItem;

            // Postprocess quad pool — mirrors Draw_Clone_Pool but clones the whole
            // quad+material pair instead of just the material.  Each draw_postprocess()
            // call gets its own quad with its own material snapshot so multiple calls
            // per frame each render with independent params (blur pass A then pass B,
            // different colors, etc.).  Quads are hidden at frame end and reused next
            // frame — no allocation after the high-water mark is reached.
            // Never exposed to Lua — behaves just like CanvasItem from the script side.
            // Single fullscreen quad parented to the active camera.
            // Renders via the normal opaque pass — has real DEPTH_TEXTURE access.
            // For sized output, the Lua script passes an RT via set_param_rt and
            // the shader writes into it; canvas then draws the RT at any size.
            // The quad is a child of the active Camera3D, so Godot frees it
            // whenever that camera is freed (scene restart, camera swap...).
            // We therefore NEVER hold raw node pointers: only ObjectIDs that are
            // re-validated through ObjectDB every time they are used.
            godot::ObjectID pp_quad_id;
            godot::Ref<godot::ShaderMaterial> pp_material;
            uint64_t pp_last_frame = UINT64_MAX;
            godot::MeshInstance3D* pp_get_quad() const;

            void postprocess_init();
            void postprocess_free();

            // Registry of every live Postprocess shader so the canvas can hide
            // quads that were not drawn this frame (i.e. the script stopped
            // calling draw_material on them, e.g. toggled off with a key).
            inline static std::vector<Shader*> postprocess_registry;

            // Per-surface material clones. A ShaderMaterial can only hold one
            // texture per uniform, so every surface that needs its own
            // original texture (albedo/normal/...) gets its own clone.
            // Parameters set via set_param* are mirrored to all live clones.
            std::vector<godot::Ref<godot::ShaderMaterial>> surface_materials;
            void prune_surface_materials();

            // Per-draw-call clone pool (CanvasItem mode only) — see shader.cpp
            struct Draw_Clone_Pool {
                std::vector<godot::Ref<godot::ShaderMaterial>> clones;
                size_t used = 0;
                uint64_t last_frame = UINT64_MAX;
                godot::Ref<godot::ShaderMaterial> next(
                    godot::Ref<godot::Shader> shader,
                    godot::Ref<godot::ShaderMaterial> source);
                void clear() { clones.clear(); used = 0; last_frame = UINT64_MAX; }
            };
            Draw_Clone_Pool draw_pool;

            struct Internal {
                struct EntryPoint {
                    std::string signature;
                    std::string usage;
                };

                static constexpr const char* SENTINEL = "vsdk_sentinel";

                
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

            // Postprocess draw — called from Canvas::draw_material(Shader*) when
            // mode == Postprocess.  Shows the quad this frame and stamps params.
            // The quad hides itself automatically the next frame if not called again.
            void draw_postprocess();

            // Called once per frame by Canvas::_process AFTER scripts have drawn.
            // Hides every postprocess quad that was not drawn during this frame.
            static void end_frame_postprocess();


            // Getters //
            Mode get_mode() const;
            std::string get_code() const;
            godot::Ref<godot::ShaderMaterial> get_material() const;
            godot::Ref<godot::ShaderMaterial> create_surface_material(godot::Ref<godot::Material> original);
            SurfaceFactory get_surface_factory();
            godot::Ref<godot::ShaderMaterial> snapshot_draw_material();


            // Setters //
            bool set_param(const std::string& name, const godot::Variant& value);
            bool set_param_rid(const std::string& name, godot::RID texture);
            bool set_param_texture(const std::string& name, godot::Ref<godot::Texture2D> texture);
            bool set_param_viewport_texture(const std::string& name, godot::Ref<godot::ViewportTexture> texture);
    };
}
#endif
