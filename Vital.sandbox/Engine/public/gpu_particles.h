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
#include <godot_cpp/classes/gpu_particles3d.hpp>
#include <godot_cpp/classes/particle_process_material.hpp>
#include <godot_cpp/classes/quad_mesh.hpp>
#include <godot_cpp/classes/ribbon_trail_mesh.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/gradient.hpp>
#include <godot_cpp/classes/gradient_texture1_d.hpp>
#include <godot_cpp/classes/curve.hpp>
#include <godot_cpp/classes/curve_texture.hpp>


///////////////////////////////////
// Vital: Engine: GPU_Particles //
///////////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    class GPU_Particles : public godot::GPUParticles3D {
        GDCLASS(GPU_Particles, godot::GPUParticles3D)
        private:
            // Built-in resources. Each emitter owns its own copies so
            // params never leak between emitters.
            godot::Ref<godot::QuadMesh> quad;
            godot::Ref<godot::RibbonTrailMesh> ribbon;
            godot::Ref<godot::StandardMaterial3D> material;
            godot::Ref<godot::ParticleProcessMaterial> process;
            godot::Ref<godot::ShaderMaterial> draw_shader;
            godot::Ref<godot::ShaderMaterial> process_shader;
            bool billboard = true;
            bool trail_mode = false;

            
            // Helpers //
            void apply_billboard();


            // Instantiators //
            GPU_Particles() = default;
            ~GPU_Particles() override = default;
            static void _bind_methods() {}
        public:
            // Managers //
            static GPU_Particles* create();
            void destroy();
            void burst();


            // Checkers //
            bool has_draw_shader() const;
            bool has_process_shader() const;
            bool is_trail_mode() const;
            bool is_billboard() const;


            // Getters //
            godot::Ref<godot::ParticleProcessMaterial> get_process_params() const;
            godot::Ref<godot::StandardMaterial3D> get_draw_material() const;
            godot::Vector2 get_draw_size() const;


            // Setters //
            void set_draw_size(const godot::Vector2& size);
            void set_draw_texture(const godot::Ref<godot::Texture2D>& texture);
            void set_billboard(bool state);
            void set_draw_shader(const godot::Ref<godot::ShaderMaterial>& shader_material);
            void set_process_shader(const godot::Ref<godot::ShaderMaterial>& shader_material);
            void set_color_ramp(const godot::Color& from, const godot::Color& to);
            void reset_color_ramp();
            void set_scale_curve(float from, float to);
            void reset_scale_curve();
            void set_trail(bool state, double lifetime);
    };
}
#endif
