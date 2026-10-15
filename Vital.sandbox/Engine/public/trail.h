/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: trail.h
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
#include <deque>
#include <Vital.sandbox/Engine/public/core.h>


// TODO: Improve
///////////////////////////
// Vital: Engine: Trail //
///////////////////////////

// A ribbon drawn along the path its own node travels. Move the node every frame (for example onto
// a bone) and the ribbon follows it: full width and color at the node, thinning out and fading with age.
// Built on the CPU from sampled positions, Godot's particle trails only follow a particle's own path
namespace Vital::Engine {
    class Trail : public godot::Node3D {
        GDCLASS(Trail, godot::Node3D)
        private:
            struct Point {
                godot::Vector3 position;
                double time;
            };

            godot::MeshInstance3D* ribbon = nullptr; // Top level child: the strip is built in world space
            godot::Ref<godot::ImmediateMesh> mesh;
            godot::Ref<godot::StandardMaterial3D> material;
            std::deque<Point> points;
            double lifetime = 0.5;
            double clock = 0.0;
            float width = 0.1f;
            godot::Color color_from = godot::Color(1, 1, 1, 1);
            godot::Color color_to = godot::Color(1, 1, 1, 0);

            // Instantiators //
            Trail() = default;
            ~Trail() override = default;
            static void _bind_methods() {}

            // Utilities //
            void rebuild();
        public:
            // Hooks //
            void _process(double delta) override;

            // Managers //
            static Trail* create();
            void destroy();
            void clear();

            // Getters //
            double get_lifetime() const;
            float get_width() const;
            godot::Color get_color_from() const;
            godot::Color get_color_to() const;
            godot::BaseMaterial3D::BlendMode get_blend_mode() const;

            // Setters //
            void set_lifetime(double seconds);
            void set_width(float size);
            void set_colors(const godot::Color& from, const godot::Color& to);
            void set_blend_mode(godot::BaseMaterial3D::BlendMode mode);
    };
}
#endif