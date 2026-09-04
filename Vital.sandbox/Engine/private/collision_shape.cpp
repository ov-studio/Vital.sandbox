/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: collision_shape.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Collision Shape Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/collision_shape.h>


/////////////////////////////////////
// Vital: Engine: Collision_Shape //
/////////////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    Collision_Shape::Collision_Shape() {
        #if defined(VSDK_Client)
        {
            std::lock_guard<std::mutex> lock(live_instances_mutex);
            live_instances.insert(this);
        }
        if (default_debug_enabled) set_debug_visible(true);
        #endif
    }

    Collision_Shape::~Collision_Shape() {
        #if defined(VSDK_Client)
        std::lock_guard<std::mutex> lock(live_instances_mutex);
        live_instances.erase(this);
        #endif
    }

    void Collision_Shape::_notification(int what) {
        if (what == NOTIFICATION_PREDELETE && on_destroyed_callback) on_destroyed_callback(this);
    }


    // Managers //
    Collision_Shape* Collision_Shape::create(godot::Node3D* owner) {
        auto body = memnew(Collision_Shape);
        if (owner) owner -> add_child(body);
        else Engine::Core::get_singleton() -> add_child(body);
        return body;
    }

    void Collision_Shape::destroy() {
        queue_free();
    }

    void Collision_Shape::assign_shape(godot::Ref<godot::Shape3D> shape) {
        set_shape(shape);
        #if defined(VSDK_Client)
        current_shape = shape;
        refresh_debug_mesh();
        #endif
    }

    bool Collision_Shape::is_replicated() const {
        auto* parent = get_parent();
        if (!parent) return false;
        auto* syncable = dynamic_cast<ISyncable*>(godot::Object::cast_to<godot::Object>(parent));
        return syncable && syncable -> get_net_id() > 0;
    }

    #if defined(VSDK_Client)
    void Collision_Shape::set_debug_visible(bool state) {
        if (state) {
            if (!debug_mesh) {
                debug_mesh = memnew(godot::MeshInstance3D);
                add_child(debug_mesh);
            }
            debug_mesh -> set_visible(true);
            refresh_debug_mesh();
        }
        else if (debug_mesh) debug_mesh -> set_visible(false);
    }

    void Collision_Shape::refresh_debug_mesh() {
        if (!debug_mesh || !current_shape.is_valid()) return;
        auto color = is_replicated() ? replicated_debug_color : local_debug_color;
        debug_mesh -> set_mesh(build_wireframe_mesh(current_shape, color));
    }

    // Wireframe geometry builders — one closed ring/line-set per shape type,
    // assembled into a PRIMITIVE_LINES mesh. Moved here (unchanged) from
    // API::Collision_Shape so the engine layer is self-contained and no
    // longer needs the API layer to hand it shape data after the fact.
    // TODO: Make internal like resource etc
    namespace {
        void add_ring(godot::PackedVector3Array& points, float radius, float y, int plane, int segments = 24) {
            // plane: 0 = XZ (horizontal ring), 1 = XY (vertical ring), 2 = YZ (vertical ring) //
            for (int i = 0; i < segments; i++) {
                float a0 = (float)i / segments * 6.28318530717958647692f;
                float a1 = (float)(i + 1) / segments * 6.28318530717958647692f;
                godot::Vector3 p0, p1;
                if (plane == 0) {
                    p0 = godot::Vector3(std::cos(a0) * radius, y, std::sin(a0) * radius);
                    p1 = godot::Vector3(std::cos(a1) * radius, y, std::sin(a1) * radius);
                }
                else if (plane == 1) {
                    p0 = godot::Vector3(std::cos(a0) * radius, std::sin(a0) * radius + y, 0);
                    p1 = godot::Vector3(std::cos(a1) * radius, std::sin(a1) * radius + y, 0);
                }
                else {
                    p0 = godot::Vector3(0, std::sin(a0) * radius + y, std::cos(a0) * radius);
                    p1 = godot::Vector3(0, std::sin(a1) * radius + y, std::cos(a1) * radius);
                }
                points.push_back(p0);
                points.push_back(p1);
            }
        }

        void add_half_ring(godot::PackedVector3Array& points, float radius, float center_y, int plane, bool upper, int segments = 12) {
            // Vertical half-circle used for capsule hemisphere caps. //
            float start = upper ? 0.0f : 3.14159265358979323846f;
            float end = upper ? 3.14159265358979323846f : 6.28318530717958647692f;
            for (int i = 0; i < segments; i++) {
                float a0 = start + (end - start) * (float)i / segments;
                float a1 = start + (end - start) * (float)(i + 1) / segments;
                godot::Vector3 p0, p1;
                if (plane == 1) {
                    p0 = godot::Vector3(std::cos(a0) * radius, std::sin(a0) * radius + center_y, 0);
                    p1 = godot::Vector3(std::cos(a1) * radius, std::sin(a1) * radius + center_y, 0);
                }
                else {
                    p0 = godot::Vector3(0, std::sin(a0) * radius + center_y, std::cos(a0) * radius);
                    p1 = godot::Vector3(0, std::sin(a1) * radius + center_y, std::cos(a1) * radius);
                }
                points.push_back(p0);
                points.push_back(p1);
            }
        }
    }

    godot::Ref<godot::ArrayMesh> Collision_Shape::build_wireframe_mesh(const godot::Ref<godot::Shape3D>& shape, const godot::Color& color) {
        godot::PackedVector3Array points;

        if (auto box = godot::Object::cast_to<godot::BoxShape3D>(shape.ptr())) {
            auto s = box -> get_size() * 0.5f;
            godot::Vector3 corners[8] = {
                {-s.x,-s.y,-s.z}, { s.x,-s.y,-s.z}, { s.x,-s.y, s.z}, {-s.x,-s.y, s.z},
                {-s.x, s.y,-s.z}, { s.x, s.y,-s.z}, { s.x, s.y, s.z}, {-s.x, s.y, s.z}
            };
            int edges[12][2] = {{0,1},{1,2},{2,3},{3,0}, {4,5},{5,6},{6,7},{7,4}, {0,4},{1,5},{2,6},{3,7}};
            for (auto& e : edges) { points.push_back(corners[e[0]]); points.push_back(corners[e[1]]); }
        }
        else if (auto sphere = godot::Object::cast_to<godot::SphereShape3D>(shape.ptr())) {
            float r = static_cast<float>(sphere -> get_radius());
            add_ring(points, r, 0, 0); add_ring(points, r, 0, 1); add_ring(points, r, 0, 2);
        }
        else if (auto capsule = godot::Object::cast_to<godot::CapsuleShape3D>(shape.ptr())) {
            float r = static_cast<float>(capsule -> get_radius());
            float half_h = std::max<float>(static_cast<float>(capsule -> get_height()) * 0.5f - r, 0.0f);
            add_ring(points, r, half_h, 0); add_ring(points, r, -half_h, 0);
            add_half_ring(points, r, half_h, 1, true); add_half_ring(points, r, half_h, 2, true);
            add_half_ring(points, r, -half_h, 1, false); add_half_ring(points, r, -half_h, 2, false);
            float side_pts[4][2] = {{r,0},{-r,0},{0,r},{0,-r}};
            for (auto& p : side_pts) {
                points.push_back(godot::Vector3(p[0], half_h, p[1]));
                points.push_back(godot::Vector3(p[0], -half_h, p[1]));
            }
        }
        else if (auto cylinder = godot::Object::cast_to<godot::CylinderShape3D>(shape.ptr())) {
            float r = static_cast<float>(cylinder -> get_radius());
            float half_h = static_cast<float>(cylinder -> get_height()) * 0.5f;
            add_ring(points, r, half_h, 0); add_ring(points, r, -half_h, 0);
            float side_pts[4][2] = {{r,0},{-r,0},{0,r},{0,-r}};
            for (auto& p : side_pts) {
                points.push_back(godot::Vector3(p[0], half_h, p[1]));
                points.push_back(godot::Vector3(p[0], -half_h, p[1]));
            }
        }

        godot::Ref<godot::ArrayMesh> mesh;
        mesh.instantiate();
        if (points.size() > 0) {
            godot::Array arrays;
            arrays.resize(godot::Mesh::ARRAY_MAX);
            arrays[godot::Mesh::ARRAY_VERTEX] = points;
            mesh -> add_surface_from_arrays(godot::Mesh::PRIMITIVE_LINES, arrays);

            godot::Ref<godot::StandardMaterial3D> material;
            material.instantiate();
            material -> set_shading_mode(godot::StandardMaterial3D::SHADING_MODE_UNSHADED);
            material -> set_albedo(color);
            mesh -> surface_set_material(0, material);
        }
        return mesh;
    }
    #endif
}
