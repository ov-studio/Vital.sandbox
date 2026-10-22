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
#include <Vital.sandbox/Engine/public/camera.h>
#include <Vital.sandbox/Engine/public/mainmenu.h>


///////////////////////////////////
// Vital: Engine: GPU_Particles //
///////////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    namespace {
        // Far plane of the active camera (already includes the draw distance), looked up once per frame for all emitters
        float get_reference_clip() {
            static uint64_t frame = UINT64_MAX;
            static float clip = 0.0f;
            const uint64_t current = godot::Engine::get_singleton() -> get_process_frames();
            if (current != frame) {
                frame = current;
                auto* camera = Camera::get_active();
                clip = camera ? camera -> get_far() : 0.0f;
            }
            return clip;
        }
    }


    // Helpers //
    // Particle distance setting: past (camera far x multiplier) Godot stops rendering the emitter.
    // At 100% the range is switched off, so the far plane stays the only limit.
    void GPU_Particles::sync_distance() {
        const float mult = MainMenu::get_particle_distance();
        float range = 0.0f;
        if (mult < 1.0f) {
            const float clip = get_reference_clip();
            if (clip <= 0.0f) return; // no camera yet: keep the current range
            range = clip*mult;
        }
        if (range == distance_range) return;
        distance_range = range;
        set_visibility_range_end(range);
    }

    // Signals //
    // An emission cycle started: fires "particle:emit"
    void GPU_Particles::notify_emit() {
        was_emitting = true;
        if (emit_handler) emit_handler();
    }

    // Every particle of a one-shot cycle is gone: fires "particle:finished"
    void GPU_Particles::notify_finished() {
        if (finished_handler) finished_handler();
    }

    void GPU_Particles::apply_billboard() {
        // Trail meshes orient themselves from the particle transform, so
        // material billboarding is forced off while trail mode is active
        material -> set_billboard_mode((billboard && !trail_mode) ? godot::BaseMaterial3D::BILLBOARD_PARTICLES : godot::BaseMaterial3D::BILLBOARD_DISABLED);
    }


    void GPU_Particles::apply_draw_passes() {
        // Trail wins over custom meshes, which win over the default quad
        if (trail_mode) {
            set_draw_passes(1);
            set_draw_pass_mesh(0, ribbon);
        }
        else if (!meshes.empty()) {
            set_draw_passes((int32_t)meshes.size());
            for (int i = 0; i < (int)meshes.size(); i++) set_draw_pass_mesh(i, meshes[i]);
        }
        else {
            set_draw_passes(1);
            set_draw_pass_mesh(0, quad);
        }
    }

    void GPU_Particles::apply_material() {
        // Quad and ribbon carry the built-in material themselves. Custom meshes keep
        // their own materials unless a texture was set, then the built-in material is forced
        if (draw_shader.is_valid()) set_material_override(draw_shader);
        else if (!trail_mode && !meshes.empty() && textured) set_material_override(material);
        else set_material_override(godot::Ref<godot::Material>());
    }


    // Hooks //
    void GPU_Particles::_process(double) {
        sync_distance();

        // Emission started by anything that bypasses emit/burst/restart_emission (set_emitting from the
        // engine, preprocess, a one-shot restarting itself): picked up on the rising edge
        const bool emitting = is_emitting();
        if (emitting && !was_emitting) notify_emit();
        was_emitting = emitting;
    }


    // Events //
    void GPU_Particles::on_finished() {
        notify_finished();
    }

    void GPU_Particles::set_emit_handler(std::function<void()> handler) {
        emit_handler = std::move(handler);
    }

    void GPU_Particles::set_finished_handler(std::function<void()> handler) {
        finished_handler = std::move(handler);
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

        // Keeps the particle distance setting applied (see sync_distance) and watches for emission starts
        particles -> set_process(true);

        // Godot's own "finished" signal: only emitted by one-shot emitters, once every particle is gone
        particles -> connect("finished", godot::Callable(particles, "_on_finished"));

        Engine::Core::get_singleton() -> add_child(particles);
        return particles;
    }

    void GPU_Particles::destroy() {
        // No signal may fire once the emitter is on its way out
        emit_handler = nullptr;
        finished_handler = nullptr;
        queue_free();
    }

    void GPU_Particles::emit() {
        // Already emitting: nothing started, so no signal
        const bool running = is_emitting();
        set_emitting(true);
        if (!running) notify_emit();
    }

    void GPU_Particles::restart_emission() {
        restart();
        notify_emit();
    }

    void GPU_Particles::burst() {
        set_one_shot(true);
        restart();
        set_emitting(true);
        notify_emit();
    }


    // Checkers //
    bool GPU_Particles::has_draw_shader() const {
        return draw_shader.is_valid();
    }

    bool GPU_Particles::has_mesh() const {
        return !meshes.empty();
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

    void GPU_Particles::set_billboard(bool state) {
        billboard = state;
        apply_billboard();
    }

    void GPU_Particles::set_material(const godot::Ref<godot::Texture2D>& texture) {
        // A texture is shown through the built-in material, so it replaces any shader
        draw_shader.unref();
        textured = texture.is_valid();
        material -> set_texture(godot::BaseMaterial3D::TEXTURE_ALBEDO, texture);
        apply_material();
    }

    void GPU_Particles::set_material(Texture* texture) {
        if (!texture) return;
        texture -> heartbeat();
        set_material(godot::Ref<godot::Texture2D>(texture -> get_texture()));
    }

    void GPU_Particles::set_material(Rendertarget* rendertarget) {
        if (!rendertarget) return;
        set_material(godot::Ref<godot::Texture2D>(rendertarget -> get_texture()));
    }

    void GPU_Particles::set_material(Shader* shader) {
        if (!shader) return;
        if (shader -> get_mode() != Shader::Mode::Spatial) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "shader must be created with the SPATIAL mode");
        draw_shader = shader -> create_surface_material(godot::Ref<godot::Material>());
        apply_material();
    }

    void GPU_Particles::set_material(const std::string& path) {
        try {
            auto texture = Texture::get_from_reference(path);
            if (!texture) texture = Image::create(path, path);
            set_material(texture);
        }
        catch (...) { std::rethrow_exception(std::current_exception()); }
    }

    void GPU_Particles::reset_material() {
        draw_shader.unref();
        textured = false;
        material -> set_texture(godot::BaseMaterial3D::TEXTURE_ALBEDO, godot::Ref<godot::Texture2D>());
        apply_material();
    }

    void GPU_Particles::set_mesh(Model* model, const std::string& component) {
        if (!model) return;
        std::vector<godot::Ref<godot::Mesh>> collected;
        if (!component.empty()) {
            auto node = model -> find_mesh_node(model, component);
            if (!node) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("component '{}' not found in model '{}'", component, model -> get_model_name()));
            if (node -> get_mesh().is_valid()) collected.push_back(node -> get_mesh());
        }
        else {
            for (const auto& path : model -> get_components()) {
                if ((int)collected.size() >= godot::GPUParticles3D::MAX_DRAW_PASSES) break;
                auto node = model -> find_mesh_node(model, path);
                if (node && node -> get_mesh().is_valid()) collected.push_back(node -> get_mesh());
            }
        }
        if (collected.empty()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("model '{}' has no meshes to use (not loaded yet?)", model -> get_model_name()));

        // Trail meshes and custom meshes are mutually exclusive draw passes
        if (trail_mode) set_trail(false, 0.0);
        meshes = collected;

        // Real 3D meshes are not billboards, set_billboard(true) turns it back on
        billboard = false;
        apply_billboard();
        apply_draw_passes();
        apply_material();
    }

    void GPU_Particles::reset_mesh() {
        meshes.clear();
        billboard = true;
        apply_billboard();
        apply_draw_passes();
        apply_material();
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
            apply_draw_passes();
            apply_material();
            set_transform_align(godot::GPUParticles3D::TRANSFORM_ALIGN_Z_BILLBOARD_Y_TO_VELOCITY);
            set_trail_lifetime(lifetime);
            set_trail_enabled(true);
        }
        else {
            set_trail_enabled(false);
            trail_mode = false;
            material -> set_flag(godot::BaseMaterial3D::FLAG_PARTICLE_TRAILS_MODE, false);
            apply_billboard();
            apply_draw_passes();
            apply_material();
            set_transform_align(godot::GPUParticles3D::TRANSFORM_ALIGN_DISABLED);
        }
    }
}
#endif
