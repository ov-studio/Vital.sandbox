/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: vehicle_wheel.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vehicle Wheel Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/vehicle_wheel.h>


///////////////////////////////////
// Vital: Engine: Vehicle_Wheel //
///////////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    // Helpers //
    #if defined(VSDK_Client)
    namespace {
        constexpr float debug_tau = 6.28318530717958647692f;

        inline void debug_line(godot::ImmediateMesh* mesh, const godot::Vector3& a, const godot::Vector3& b, const godot::Color& color_a, const godot::Color& color_b) {
            mesh -> surface_set_color(color_a);
            mesh -> surface_add_vertex(a);
            mesh -> surface_set_color(color_b);
            mesh -> surface_add_vertex(b);
        }

        inline void debug_line(godot::ImmediateMesh* mesh, const godot::Vector3& a, const godot::Vector3& b, const godot::Color& color) {
            debug_line(mesh, a, b, color, color);
        }

        inline void debug_cross(godot::ImmediateMesh* mesh, const godot::Vector3& center, float size, const godot::Color& color) {
            debug_line(mesh, center - godot::Vector3(size, 0, 0), center + godot::Vector3(size, 0, 0), color);
            debug_line(mesh, center - godot::Vector3(0, size, 0), center + godot::Vector3(0, size, 0), color);
            debug_line(mesh, center - godot::Vector3(0, 0, size), center + godot::Vector3(0, 0, size), color);
        }
    }
    #endif


    // Instantiators //
    Vehicle_Wheel::Vehicle_Wheel() {
        #if defined(VSDK_Client)
        std::lock_guard<std::mutex> lock(mutex);
        buffer.insert(this);
        #endif
    }

    Vehicle_Wheel::~Vehicle_Wheel() {
        #if defined(VSDK_Client)
        std::lock_guard<std::mutex> lock(mutex);
        buffer.erase(this);
        #endif
    }


    // Hooks //
    void Vehicle_Wheel::_notification(int what) {
        #if defined(VSDK_Client)
        if (what == NOTIFICATION_ENTER_TREE) {
            // Godot's VehicleWheel3D captures its connection point + axes from its transform when it enters the tree
            // (and again on every re-parent, see refresh()). Mirror that so the suspension debug matches the real one.
            has_rest_transform = godot::Object::cast_to<godot::VehicleBody3D>(get_parent()) != nullptr;
            if (has_rest_transform) rest_transform = get_transform();
            if (debug_all && !debug_visible) set_debug_visible(true);
        }
        #endif
        if (what == NOTIFICATION_PREDELETE) Tool::Event::emit("entity:unspawn", Tool::Stack({this}));
    }

    #if defined(VSDK_Client)
    void Vehicle_Wheel::_process(double delta) {
        update_debug();
    }
    #endif


    // Managers //
    Vehicle_Wheel* Vehicle_Wheel::create(godot::Node3D* body) {
        auto wheel = memnew(Vehicle_Wheel);
        if (body) body -> add_child(wheel);
        else Engine::Core::get_singleton() -> add_child(wheel);
        if (auto vehicle = godot::Object::cast_to<godot::VehicleBody3D>(body)) wheel -> set_brake(vehicle -> get_brake());
        wheel -> sync_inputs();
        Tool::Event::emit("entity:spawn", Tool::Stack({wheel, false}));
        return wheel;
    }

    void Vehicle_Wheel::destroy() {
        queue_free();
    }

    void Vehicle_Wheel::refresh() {
        const godot::ObjectID oid(get_instance_id());
        Engine::Core::get_singleton() -> execute([oid]() {
            auto wheel = godot::Object::cast_to<Vehicle_Wheel>(godot::ObjectDB::get_instance(oid));
            if (!wheel || !wheel -> is_inside_tree()) return;
            auto parent = wheel -> get_parent();
            if (!parent) return;
            const int index = wheel -> get_index();
            parent -> remove_child(wheel);
            parent -> add_child(wheel);
            parent -> move_child(wheel, index);
        });
    }

    void Vehicle_Wheel::sync_inputs() {
        auto vehicle = godot::Object::cast_to<godot::VehicleBody3D>(get_parent());
        if (!vehicle) return;
        if (is_used_as_traction()) set_engine_force(vehicle -> get_engine_force());
        if (is_used_as_steering()) set_steering(vehicle -> get_steering());
    }


    // Checkers //
    #if defined(VSDK_Client)
    bool Vehicle_Wheel::is_debug_visible() const {
        return debug_visible;
    }

    bool Vehicle_Wheel::is_debug_all() {
        return debug_all;
    }
    #endif


    // Getters //
    int Vehicle_Wheel::get_wheel_id() const {
        return wheel_id;
    }

    uint32_t Vehicle_Wheel::get_parent_net_id() const {
        return Engine::get_parent_net_id(this);
    }


    // Setters //
    void Vehicle_Wheel::set_wheel_id(int index) {
        wheel_id = index;
    }

    #if defined(VSDK_Client)
    void Vehicle_Wheel::set_debug_visible(bool state) {
        debug_visible = state;
        if (state) {
            if (!debug_material.is_valid()) {
                debug_material.instantiate();
                debug_material -> set_shading_mode(godot::StandardMaterial3D::SHADING_MODE_UNSHADED);
                debug_material -> set_flag(godot::StandardMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
                debug_material -> set_flag(godot::StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
                debug_material -> set_albedo(godot::Color(1, 1, 1));
                debug_material -> set_render_priority(64);
            }
            if (!debug_wheel_node) {
                debug_wheel_mesh.instantiate();
                debug_wheel_node = memnew(godot::MeshInstance3D);
                debug_wheel_node -> set_name("__debug_wheel");
                debug_wheel_node -> set_mesh(debug_wheel_mesh);
                debug_wheel_node -> set_cast_shadows_setting(godot::GeometryInstance3D::SHADOW_CASTING_SETTING_OFF);
                add_child(debug_wheel_node);
            }
            if (!debug_suspension_node) {
                debug_suspension_mesh.instantiate();
                debug_suspension_node = memnew(godot::MeshInstance3D);
                debug_suspension_node -> set_name("__debug_suspension");
                debug_suspension_node -> set_mesh(debug_suspension_mesh);
                debug_suspension_node -> set_cast_shadows_setting(godot::GeometryInstance3D::SHADOW_CASTING_SETTING_OFF);
                // Lives in body space (suspension does not roll/steer with the wheel), so it ignores the wheel's own transform
                debug_suspension_node -> set_as_top_level(true);
                add_child(debug_suspension_node);
            }
            debug_wheel_node -> set_visible(true);
            debug_suspension_node -> set_visible(true);
            set_process(true);
            update_debug();
        }
        else {
            if (debug_wheel_node) debug_wheel_node -> set_visible(false);
            if (debug_suspension_node) debug_suspension_node -> set_visible(false);
            set_process(false);
        }
    }

    void Vehicle_Wheel::set_debug_all(bool state) {
        debug_all = state;
        std::lock_guard<std::mutex> lock(mutex);
        for (auto wheel : buffer) wheel -> set_debug_visible(state);
    }
    #endif


    // Misc //
    #if defined(VSDK_Client)
    void Vehicle_Wheel::update_debug() {
        if (!debug_visible || !debug_wheel_node || !debug_suspension_node || !is_inside_tree()) return;

        const float radius = static_cast<float>(get_radius());
        const bool contact = is_in_contact();
        const float skid = std::max(0.0f, std::min(1.0f, static_cast<float>(get_skidinfo())));

        // Wheel: ring + spokes + axle in wheel space (axle = local X). Rolls and steers with the wheel.
        // Ring colour: cyan = full grip, towards red = skidding, magenta = airborne. The yellow spoke makes spin visible.
        {
            const godot::Color ring_color = contact ? godot::Color(1.0f, 0.15f, 0.15f).lerp(godot::Color(0.0f, 1.0f, 1.0f), skid) : godot::Color(1.0f, 0.0f, 1.0f);
            const godot::Color spoke_marker_color(1.0f, 1.0f, 0.0f);
            const godot::Color axle_color(0.85f, 0.85f, 0.85f);
            const int segments = 32;
            const int spokes = 6;
            const float axle_half = std::max(0.1f, radius * 0.35f);

            auto mesh = debug_wheel_mesh.ptr();
            mesh -> clear_surfaces();
            mesh -> surface_begin(godot::Mesh::PRIMITIVE_LINES, debug_material);
            for (int i = 0; i < segments; i++) {
                const float a0 = (float)i / segments * debug_tau;
                const float a1 = (float)(i + 1) / segments * debug_tau;
                debug_line(mesh, godot::Vector3(0, std::cos(a0) * radius, std::sin(a0) * radius), godot::Vector3(0, std::cos(a1) * radius, std::sin(a1) * radius), ring_color);
            }
            for (int i = 0; i < spokes; i++) {
                const float a = (float)i / spokes * debug_tau;
                debug_line(mesh, godot::Vector3(), godot::Vector3(0, std::cos(a) * radius, std::sin(a) * radius), i == 0 ? spoke_marker_color : ring_color);
            }
            debug_line(mesh, godot::Vector3(-axle_half, 0, 0), godot::Vector3(axle_half, 0, 0), axle_color);
            mesh -> surface_end();
        }

        // Suspension: drawn in body space, anchored at the wheel's connection point
        //   white cross   = connection point (suspension hard point)
        //   orange line   = raycast (rest length + radius, i.e. down to the wheel's bottom at rest)
        //   blue bar      = suspension travel range for the wheel centre (rest - travel .. rest)
        //   green -> red  = hard point to current wheel centre, red as the spring compresses to its travel limit
        //   red cross     = contact point (+ surface normal), grey line = wheel centre to contact point
        auto parent = godot::Object::cast_to<godot::Node3D>(get_parent());
        if (!parent || !has_rest_transform) {
            debug_suspension_node -> set_visible(false);
            return;
        }
        debug_suspension_node -> set_visible(true);

        const godot::Vector3 hard_point = rest_transform.origin;
        const godot::Transform3D node_xform = parent -> get_global_transform() * godot::Transform3D(godot::Basis(), hard_point);
        debug_suspension_node -> set_global_transform(node_xform);

        const godot::Vector3 dir = -rest_transform.basis.get_column(1).normalized();
        const godot::Vector3 axle = rest_transform.basis.get_column(0).normalized();
        const float rest = static_cast<float>(get_suspension_rest_length());
        const float travel = static_cast<float>(get_suspension_travel());
        const godot::Vector3 center = get_position() - hard_point;
        const float length = center.dot(dir);
        const float compression = travel > 0.0001f ? std::max(0.0f, std::min(1.0f, (rest - length) / travel)) : 0.0f;
        const godot::Color spring_color = godot::Color(0.2f, 1.0f, 0.2f).lerp(godot::Color(1.0f, 0.2f, 0.2f), compression);
        const godot::Color white(1.0f, 1.0f, 1.0f);
        const godot::Color orange(1.0f, 0.6f, 0.0f);
        const godot::Color blue(0.3f, 0.5f, 1.0f);
        const godot::Color red(1.0f, 0.1f, 0.1f);

        auto mesh = debug_suspension_mesh.ptr();
        mesh -> clear_surfaces();
        mesh -> surface_begin(godot::Mesh::PRIMITIVE_LINES, debug_material);
        debug_cross(mesh, godot::Vector3(), 0.08f, white);

        const godot::Vector3 ray_end = dir * (rest + radius);
        debug_line(mesh, godot::Vector3(), ray_end, orange);
        debug_line(mesh, ray_end - axle * 0.06f, ray_end + axle * 0.06f, orange);

        const godot::Vector3 travel_offset = axle * 0.08f;
        const godot::Vector3 travel_min = dir * (rest - travel) + travel_offset;
        const godot::Vector3 travel_max = dir * rest + travel_offset;
        debug_line(mesh, travel_min, travel_max, blue);
        debug_line(mesh, travel_min - axle * 0.04f, travel_min + axle * 0.04f, blue);
        debug_line(mesh, travel_max - axle * 0.04f, travel_max + axle * 0.04f, blue);

        debug_line(mesh, godot::Vector3(), center, spring_color);
        debug_cross(mesh, center, 0.06f, spring_color);

        if (contact) {
            const godot::Transform3D inverse = node_xform.affine_inverse();
            const godot::Vector3 point = inverse.xform(get_contact_point());
            const godot::Vector3 normal = inverse.basis.xform(get_contact_normal()).normalized();
            debug_line(mesh, center, point, godot::Color(0.5f, 0.5f, 0.5f));
            debug_cross(mesh, point, 0.08f, red);
            debug_line(mesh, point, point + normal * 0.5f, red, godot::Color(1.0f, 1.0f, 0.0f));
        }
        mesh -> surface_end();
    }
    #endif
}
