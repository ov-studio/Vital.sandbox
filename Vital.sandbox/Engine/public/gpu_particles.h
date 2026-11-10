/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: gpu_particles.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 11/10/2026
     Desc: GPU Particles Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/texture.h>
#include <Vital.sandbox/Engine/public/image.h>
#include <Vital.sandbox/Engine/public/rendertarget.h>
#include <Vital.sandbox/Engine/public/shader.h>
#include <Vital.sandbox/Engine/public/model.h>


///////////////////////////////////
// Vital: Engine: GPU_Particles //
///////////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    class GPU_Particles : public godot::GPUParticles3D, public Engine::Entity<GPU_Particles> {
        GDCLASS(GPU_Particles, godot::GPUParticles3D)
        private:
            // Built-in resources. Each emitter owns its own copies so
            // params never leak between emitters.
            godot::Ref<godot::QuadMesh> quad;
            godot::Ref<godot::RibbonTrailMesh> ribbon;
            godot::Ref<godot::StandardMaterial3D> material;
            godot::Ref<godot::ParticleProcessMaterial> process;
            godot::Ref<godot::ShaderMaterial> draw_shader;
            std::vector<godot::Ref<godot::Mesh>> meshes;
            bool textured = false;
            bool billboard = true;
            bool trail_mode = false;
            float distance_range = 0.0f; // Applied cull distance, 0 = no limit

            // Signals: the sandbox layer wires these (see API/particle/gpu_particles.h)
            std::function<void()> emit_handler;
            std::function<void()> finished_handler;
            bool was_emitting = false;     // Last observed emitting state, used to detect emission starts

            // Helpers //
            void notify_emit();
            void notify_finished();
            void apply_billboard();
            void apply_draw_passes();
            void apply_material();
            void sync_distance();

            // World collision: one height field collider shared by every emitter. It follows the camera and
            // rasterizes the visible level from above, so no baking and no map setup is needed
            inline static godot::GPUParticlesCollisionHeightField3D* world_collider = nullptr;
            inline static godot::Vector3 world_collider_size = godot::Vector3(48, 60, 48);
            inline static int world_collider_resolution = 1024;
            static void update_world_collider();


            // Instantiators //
            GPU_Particles() = default;
            ~GPU_Particles() override = default;
            static void _bind_methods() {
                godot::ClassDB::bind_method(godot::D_METHOD("_on_finished"), &GPU_Particles::on_finished);
            }
        public:
            // Hooks //
            void _process(double delta) override;


            // Events //
            // on_finished is the target of Godot's GPUParticles3D "finished" signal
            void on_finished();
            void set_emit_handler(std::function<void()> handler);
            void set_finished_handler(std::function<void()> handler);


            // Managers //
            static GPU_Particles* create();
            void destroy();
            void emit();
            void restart_emission();
            void burst();


            // World collision (shared collider, see update_world_collider) //
            // size: x/z = metres covered around the camera, y = height of the volume centred on the camera
            // resolution: height map size in pixels (256, 512, 1024, 2048, 4096 or 8192)
            static void set_world_collision(bool state, const godot::Vector3& size, int resolution);
            static bool has_world_collision();

            // Per emitter: collide with the world collider, spawn another emitter's particles on contact
            void set_collision(godot::ParticleProcessMaterial::CollisionMode mode, float bounce, float friction);
            void attach_sub_emitter(GPU_Particles* target, godot::ParticleProcessMaterial::SubEmitterMode mode, int amount, float frequency);
            void detach_sub_emitter();


            // Checkers //
            bool has_draw_shader() const;
            bool has_mesh() const;
            bool is_trail_mode() const;
            bool is_billboard() const;


            // Getters //
            godot::Ref<godot::ParticleProcessMaterial> get_process_params() const;
            godot::Ref<godot::StandardMaterial3D> get_draw_material() const;
            godot::Vector2 get_draw_size() const;


            // Setters //
            void set_draw_size(const godot::Vector2& size);
            void set_billboard(bool state);

            // Material: a texture (image / svg / path / rendertarget) is shown through the
            // built-in material, a SPATIAL shader replaces the built-in material entirely
            void set_material(Engine::Texture* texture);
            void set_material(Engine::Rendertarget* rendertarget);
            void set_material(Engine::Shader* shader);
            void set_material(const std::string& path);
            void set_material(const godot::Ref<godot::Texture2D>& texture);
            void reset_material();

            // Mesh: replaces the quad with the meshes of a model (up to MAX_DRAW_PASSES)
            void set_mesh(Engine::Model* model, const std::string& component = "");
            void reset_mesh();

            void set_color_ramp(const godot::Color& from, const godot::Color& to);
            void reset_color_ramp();
            void set_scale_curve(float from, float to);
            void reset_scale_curve();
            void set_trail(bool state, double lifetime);
    };
}
#endif
