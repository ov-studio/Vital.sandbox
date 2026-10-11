/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: gpu_particles.cpp
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
#include <Vital.sandbox/Engine/public/gpu_particles.h>
#include <algorithm>


///////////////////////////////////
// Vital: Engine: GPU_Particles //
///////////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    // Helpers //
    void GPU_Particles::apply_billboard() {
        // Trail meshes orient themselves from the particle transform, so
        // material billboarding is forced off while trail mode is active
        material -> set_billboard_mode((billboard && !trail_mode) ? godot::BaseMaterial3D::BILLBOARD_PARTICLES : godot::BaseMaterial3D::BILLBOARD_DISABLED);
    }


    // Managers //
    GPU_Particles* GPU_Particles::create() {
        auto particles = memnew(GPU_Particles);

        // Draw pass: a camera facing quad, tinted by the particle color
        particles -> quad.instantiate();
        particles -> quad -> set_size(godot::Vector2(1, 1));
        particles -> material.instantiate();
        particles -> material -> set_shading_mode(godot::BaseMaterial3D::SHADING_MODE_UNSHADED);
        particles -> material -> set_transparency(godot::BaseMaterial3D::TRANSPARENCY_ALPHA);
        particles -> material -> set_flag(godot::BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
        particles -> material -> set_cull_mode(godot::BaseMaterial3D::CULL_DISABLED);
        particles -> apply_billboard();
        particles -> quad -> set_material(particles -> material);

        // Process pass: emits upwards so a bare create() is visible straight away
        particles -> process.instantiate();
        particles -> process -> set_direction(godot::Vector3(0, 1, 0));
        particles -> process -> set_spread(25.0f);
        particles -> process -> set_param_min(godot::ParticleProcessMaterial::PARAM_INITIAL_LINEAR_VELOCITY, 1.0f);
        particles -> process -> set_param_max(godot::ParticleProcessMaterial::PARAM_INITIAL_LINEAR_VELOCITY, 2.0f);
        particles -> process -> set_gravity(godot::Vector3(0, 0, 0));

        particles -> set_draw_passes(1);
        particles -> set_draw_pass_mesh(0, particles -> quad);
        particles -> set_process_material(particles -> process);

        // The default AABB is tiny: particles travelling outside of it get culled
        particles -> set_visibility_aabb(godot::AABB(godot::Vector3(-10, -10, -10), godot::Vector3(20, 20, 20)));
        particles -> set_cast_shadows_setting(godot::GeometryInstance3D::SHADOW_CASTING_SETTING_OFF);

        Engine::Core::get_singleton() -> add_child(particles);
        return particles;
    }

    void GPU_Particles::destroy() {
        queue_free();
    }

    void GPU_Particles::burst() {
        set_one_shot(true);
        restart();
        set_emitting(true);
    }


    // Checkers //
    bool GPU_Particles::has_draw_shader() const {
        return draw_shader.is_valid();
    }

    bool GPU_Particles::is_trail_mode() const {
        return trail_mode;
    }

    bool GPU_Particles::is_billboard() const {
        return billboard;
    }


    // Getters //
    godot::Ref<godot::ParticleProcessMaterial> GPU_Particles::get_process_params() const {
        return process;
    }

    godot::Ref<godot::StandardMaterial3D> GPU_Particles::get_draw_material() const {
        return material;
    }

    godot::Vector2 GPU_Particles::get_draw_size() const {
        return quad -> get_size();
    }


    // Setters //
    void GPU_Particles::set_draw_size(const godot::Vector2& size) {
        quad -> set_size(size);
        if (ribbon.is_valid()) ribbon -> set_size(size.x);
    }

    void GPU_Particles::set_draw_texture(const godot::Ref<godot::Texture2D>& texture) {
        material -> set_texture(godot::BaseMaterial3D::TEXTURE_ALBEDO, texture);
    }

    void GPU_Particles::set_billboard(bool state) {
        billboard = state;
        apply_billboard();
    }

    void GPU_Particles::set_draw_shader(const godot::Ref<godot::ShaderMaterial>& shader_material) {
        draw_shader = shader_material;
        set_material_override(shader_material);
    }

    void GPU_Particles::set_color_ramp(const godot::Color& from, const godot::Color& to) {
        godot::Ref<godot::Gradient> gradient;
        gradient.instantiate();
        gradient -> set_color(0, from);
        gradient -> set_color(1, to);
        godot::Ref<godot::GradientTexture1D> ramp;
        ramp.instantiate();
        ramp -> set_gradient(gradient);
        process -> set_color_ramp(ramp);
    }

    void GPU_Particles::reset_color_ramp() {
        process -> set_color_ramp(godot::Ref<godot::Texture2D>());
    }

    void GPU_Particles::set_scale_curve(float from, float to) {
        // Curves clamp points to their value range, so widen it before adding points
        godot::Ref<godot::Curve> curve;
        curve.instantiate();
        curve -> set_min_value(0.0f);
        curve -> set_max_value(std::max({from, to, 1.0f}));
        curve -> clear_points();
        curve -> add_point(godot::Vector2(0, from));
        curve -> add_point(godot::Vector2(1, to));
        godot::Ref<godot::CurveTexture> texture;
        texture.instantiate();
        texture -> set_curve(curve);
        process -> set_param_texture(godot::ParticleProcessMaterial::PARAM_SCALE, texture);
    }

    void GPU_Particles::reset_scale_curve() {
        process -> set_param_texture(godot::ParticleProcessMaterial::PARAM_SCALE, godot::Ref<godot::Texture2D>());
    }

    void GPU_Particles::set_trail(bool state, double lifetime) {
        if (state) {
            if (!ribbon.is_valid()) {
                ribbon.instantiate();
                ribbon -> set_size(quad -> get_size().x);
                ribbon -> set_sections(4);
                ribbon -> set_section_length(0.2f);
                ribbon -> set_section_segments(1);
                ribbon -> set_material(material);
            }
            trail_mode = true;
            material -> set_flag(godot::BaseMaterial3D::FLAG_PARTICLE_TRAILS_MODE, true);
            apply_billboard();
            set_draw_pass_mesh(0, ribbon);
            set_transform_align(godot::GPUParticles3D::TRANSFORM_ALIGN_Z_BILLBOARD_Y_TO_VELOCITY);
            set_trail_lifetime(lifetime);
            set_trail_enabled(true);
        }
        else {
            set_trail_enabled(false);
            trail_mode = false;
            material -> set_flag(godot::BaseMaterial3D::FLAG_PARTICLE_TRAILS_MODE, false);
            apply_billboard();
            set_draw_pass_mesh(0, quad);
            set_transform_align(godot::GPUParticles3D::TRANSFORM_ALIGN_DISABLED);
        }
    }
}
#endif
