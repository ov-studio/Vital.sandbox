/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: trail.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 15/10/2026
     Desc: Trail Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/trail.h>
#include <Vital.sandbox/Engine/public/camera.h>
#include <Vital.sandbox/Engine/public/mainmenu.h>


// TODO: Improve
///////////////////////////
// Vital: Engine: Trail //
///////////////////////////

namespace Vital::Engine {
    namespace {
        constexpr double MIN_SPACING = 0.01;  // Distance between samples
        constexpr double TELEPORT    = 10.0;  // A jump this long breaks the ribbon instead of stretching it
        constexpr size_t MAX_SAMPLES = 512;
        constexpr int    SUBDIVISIONS = 3;    // Smoothing subdivisions per segment
    }

    // Utilities //
    // Samples the node position, drops samples older than the lifetime and rebuilds the ribbon:
    // a camera facing strip, full width and color at the node, thinning out and fading with age
    void Trail::rebuild() {
        const godot::Vector3 head = get_global_position();
        if (!points.empty() && head.distance_to(points.back().position) > TELEPORT) points.clear();
        if (points.empty() || head.distance_to(points.back().position) >= MIN_SPACING) {
            points.push_back({head, clock});
            if (points.size() > MAX_SAMPLES) points.pop_front();
        }
        while (!points.empty() && clock - points.front().time > lifetime) points.pop_front();

        // Samples + the live node position, so the ribbon always touches the node
        std::vector<Point> samples(points.begin(), points.end());
        if (samples.empty() || head.distance_to(samples.back().position) > 0.0001) samples.push_back({head, clock});
        mesh -> clear_surfaces();
        if (samples.size() < 2) return;

        // Catmull-Rom smoothing: samples sit one frame apart, which shows as corners on a fast arc
        std::vector<Point> path;
        path.reserve(samples.size()*SUBDIVISIONS + 1);
        for (size_t i = 0; i + 1 < samples.size(); i++) {
            const auto& p0 = samples[i == 0 ? 0 : i - 1];
            const auto& p1 = samples[i];
            const auto& p2 = samples[i + 1];
            const auto& p3 = samples[i + 2 < samples.size() ? i + 2 : i + 1];
            for (int step = 0; step < SUBDIVISIONS; step++) {
                const float t = (float)step/(float)SUBDIVISIONS;
                path.push_back({p1.position.cubic_interpolate(p2.position, p0.position, p3.position, t), p1.time + (p2.time - p1.time)*t});
            }
        }
        path.push_back(samples.back());

        auto* viewport = get_viewport();
        auto* camera = viewport ? viewport -> get_camera_3d() : nullptr;
        const godot::Vector3 eye = camera ? camera -> get_global_position() : godot::Vector3();
        const float half = width*0.5f;

        mesh -> surface_begin(godot::Mesh::PRIMITIVE_TRIANGLE_STRIP, material);
        for (size_t i = 0; i < path.size(); i++) {
            const auto& current = path[i];
            const godot::Vector3 tangent = path[i + 1 < path.size() ? i + 1 : i].position - path[i == 0 ? 0 : i - 1].position;
            const float age = (float)std::clamp((clock - current.time)/lifetime, 0.0, 1.0);

            // Sideways: across the path, facing the camera
            godot::Vector3 side = tangent.cross(camera ? eye - current.position : godot::Vector3(0, 1, 0));
            if (side.length_squared() < 1e-8f) side = tangent.cross(godot::Vector3(0, 1, 0)); // Looking straight down the path
            if (side.length_squared() < 1e-8f) side = godot::Vector3(1, 0, 0);
            side = side.normalized()*(half*(1.0f - age));

            const godot::Color color = color_from.lerp(color_to, age);
            mesh -> surface_set_color(color);
            mesh -> surface_set_uv(godot::Vector2(age, 0.0f));
            mesh -> surface_add_vertex(current.position + side);
            mesh -> surface_set_color(color);
            mesh -> surface_set_uv(godot::Vector2(age, 1.0f));
            mesh -> surface_add_vertex(current.position - side);
        }
        mesh -> surface_end();
    }

    // Hooks //
    void Trail::_process(double delta) {
        clock += delta;

        // Hidden (dimension, visibility): nothing to draw and no stale samples to show on return
        if (!is_visible_in_tree()) {
            clear();
            return;
        }

        // Particle distance setting: past (camera far x multiplier) from the camera the ribbon is skipped.
        // Measured from the trail node: the ribbon is a top level mesh at the world origin, so a visibility range would measure from the wrong place.
        // At 100% there is no extra limit, the camera far plane stays the only one
        const float mult = Engine::MainMenu::get_particle_distance();
        if (mult < 1.0f) {
            auto camera = Engine::Camera::get_active();
            if (camera) {
                const float range = camera -> get_far()*mult;
                if (get_global_position().distance_squared_to(camera -> get_global_position()) > range*range) {
                    clear();
                    return;
                }
            }
        }
        rebuild();
    }

    // Managers //
    Trail* Trail::create() {
        auto trail = memnew(Trail);
        trail -> mesh.instantiate();
        trail -> material.instantiate();
        trail -> material -> set_shading_mode(godot::BaseMaterial3D::SHADING_MODE_UNSHADED);
        trail -> material -> set_transparency(godot::BaseMaterial3D::TRANSPARENCY_ALPHA);
        trail -> material -> set_flag(godot::BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
        trail -> material -> set_cull_mode(godot::BaseMaterial3D::CULL_DISABLED);

        // Top level: the ribbon is built in world space, so it must not follow the node's transform
        trail -> ribbon = memnew(godot::MeshInstance3D);
        trail -> ribbon -> set_mesh(trail -> mesh);
        trail -> ribbon -> set_cast_shadows_setting(godot::GeometryInstance3D::SHADOW_CASTING_SETTING_OFF);
        trail -> ribbon -> set_as_top_level(true);
        trail -> add_child(trail -> ribbon);

        trail -> set_process(true);
        Engine::Core::get_singleton() -> add_child(trail);
        return trail;
    }

    void Trail::destroy() {
        queue_free();
    }

    // Drops the current ribbon, e.g. after repositioning the node
    void Trail::clear() {
        points.clear();
        if (mesh.is_valid()) mesh -> clear_surfaces();
    }

    // Getters //
    double Trail::get_lifetime() const {
        return lifetime;
    }

    float Trail::get_width() const {
        return width;
    }

    godot::Color Trail::get_color_from() const {
        return color_from;
    }

    godot::Color Trail::get_color_to() const {
        return color_to;
    }

    godot::BaseMaterial3D::BlendMode Trail::get_blend_mode() const {
        return material -> get_blend_mode();
    }

    // Setters //
    void Trail::set_lifetime(double seconds) {
        lifetime = std::max(0.01, seconds);
    }

    void Trail::set_width(float size) {
        width = std::max(0.0f, size);
    }

    void Trail::set_colors(const godot::Color& from, const godot::Color& to) {
        color_from = from;
        color_to = to;
    }

    void Trail::set_blend_mode(godot::BaseMaterial3D::BlendMode mode) {
        material -> set_blend_mode(mode);
    }
}
#endif