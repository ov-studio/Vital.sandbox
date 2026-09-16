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

#if defined(VSDK_Client)
namespace Vital::Engine {
    // Helpers //
    void Collision_Shape::Internal::add_ring(godot::PackedVector3Array& points, float radius, float y, int plane, int segments) {
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

    void Collision_Shape::Internal::add_half_ring(godot::PackedVector3Array& points, float radius, float center_y, int plane, bool upper, int segments) {
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

    godot::Ref<godot::ArrayMesh> Collision_Shape::Internal::build_wireframe_mesh(const godot::Ref<godot::Shape3D>& shape, const godot::Color& color) {
        godot::PackedVector3Array points;
        if (auto box = godot::Object::cast_to<godot::BoxShape3D>(shape.ptr())) {
            auto s = box -> get_size() * 0.5f;
            godot::Vector3 corners[8] = {
                {-s.x,-s.y,-s.z}, { s.x,-s.y,-s.z}, { s.x,-s.y, s.z}, {-s.x,-s.y, s.z},
                {-s.x, s.y,-s.z}, { s.x, s.y,-s.z}, { s.x, s.y, s.z}, {-s.x, s.y, s.z}
            };
            int edges[12][2] = {{0,1},{1,2},{2,3},{3,0}, {4,5},{5,6},{6,7},{7,4}, {0,4},{1,5},{2,6},{3,7}};
            for (auto& e : edges) {
                points.push_back(corners[e[0]]);
                points.push_back(corners[e[1]]);
            }
        }
        else if (auto sphere = godot::Object::cast_to<godot::SphereShape3D>(shape.ptr())) {
            float r = static_cast<float>(sphere -> get_radius());
            add_ring(points, r, 0, 0);
            add_ring(points, r, 0, 1);
            add_ring(points, r, 0, 2);
        }
        else if (auto capsule = godot::Object::cast_to<godot::CapsuleShape3D>(shape.ptr())) {
            float r = static_cast<float>(capsule -> get_radius());
            float half_h = std::max<float>(static_cast<float>(capsule -> get_height()) * 0.5f - r, 0.0f);
            add_ring(points, r,  half_h, 0);
            add_ring(points, r, -half_h, 0);
            add_half_ring(points, r,  half_h, 1, true);
            add_half_ring(points, r,  half_h, 2, true);
            add_half_ring(points, r, -half_h, 1, false);
            add_half_ring(points, r, -half_h, 2, false);
            float side_pts[4][2] = {{r,0},{-r,0},{0,r},{0,-r}};
            for (auto& p : side_pts) {
                points.push_back(godot::Vector3(p[0],  half_h, p[1]));
                points.push_back(godot::Vector3(p[0], -half_h, p[1]));
            }
        }
        else if (auto cylinder = godot::Object::cast_to<godot::CylinderShape3D>(shape.ptr())) {
            float r = static_cast<float>(cylinder -> get_radius());
            float half_h = static_cast<float>(cylinder -> get_height()) * 0.5f;
            add_ring(points, r,  half_h, 0);
            add_ring(points, r, -half_h, 0);
            float side_pts[4][2] = {{r,0},{-r,0},{0,r},{0,-r}};
            for (auto& p : side_pts) {
                points.push_back(godot::Vector3(p[0],  half_h, p[1]));
                points.push_back(godot::Vector3(p[0], -half_h, p[1]));
            }
        }
        else if (auto convex = godot::Object::cast_to<godot::ConvexPolygonShape3D>(shape.ptr())) {
            auto pts = convex -> get_points();
            for (int i = 0; i < pts.size(); i++) {
                for (int j = i + 1; j < pts.size() && j < i + 4; j++) {
                    points.push_back(pts[i]);
                    points.push_back(pts[j]);
                }
            }
        }
        else if (auto concave = godot::Object::cast_to<godot::ConcavePolygonShape3D>(shape.ptr())) {
            auto faces = concave -> get_faces();
            for (int i = 0; i + 2 < faces.size(); i += 3) {
                points.push_back(faces[i]);     points.push_back(faces[i + 1]);
                points.push_back(faces[i + 1]); points.push_back(faces[i + 2]);
                points.push_back(faces[i + 2]); points.push_back(faces[i]);
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

    // TODO: WIP
    // Mesh shape builders //
    godot::Ref<godot::ConvexPolygonShape3D> Collision_Shape::Internal::build_convex_shape(godot::MeshInstance3D* mesh_instance) {
        if (!mesh_instance) return {};
        auto mesh = mesh_instance -> get_mesh();
        if (!mesh.is_valid()) return {};

        godot::PackedVector3Array verts;
        for (int s = 0; s < mesh -> get_surface_count(); s++) {
            auto arrays = mesh -> surface_get_arrays(s);
            auto surface_verts = static_cast<godot::PackedVector3Array>(arrays[godot::Mesh::ARRAY_VERTEX]);
            for (int i = 0; i < surface_verts.size(); i++) verts.push_back(surface_verts[i]);
        }
        godot::Ref<godot::ConvexPolygonShape3D> shape;
        shape.instantiate();
        shape -> set_points(verts);
        return shape;
    }

    godot::Ref<godot::ConcavePolygonShape3D> Collision_Shape::Internal::build_concave_shape(godot::MeshInstance3D* mesh_instance) {
        if (!mesh_instance) return {};
        auto mesh = mesh_instance -> get_mesh();
        if (!mesh.is_valid()) return {};
        
        godot::PackedVector3Array faces;
        for (int s = 0; s < mesh -> get_surface_count(); s++) {
            auto arrays = mesh -> surface_get_arrays(s);
            auto verts = static_cast<godot::PackedVector3Array>(arrays[godot::Mesh::ARRAY_VERTEX]);
            if (mesh -> surface_get_primitive_type(s) != godot::Mesh::PRIMITIVE_TRIANGLES) continue;
            if (arrays[godot::Mesh::ARRAY_INDEX].get_type() != godot::Variant::NIL) {
                auto indices = static_cast<godot::PackedInt32Array>(arrays[godot::Mesh::ARRAY_INDEX]);
                for (int i = 0; i + 2 < indices.size(); i += 3) {
                    faces.push_back(verts[indices[i]]);
                    faces.push_back(verts[indices[i + 1]]);
                    faces.push_back(verts[indices[i + 2]]);
                }
            }
            else {
                for (int i = 0; i < verts.size(); i++) faces.push_back(verts[i]);
            }
        }
        godot::Ref<godot::ConcavePolygonShape3D> shape;
        shape.instantiate();
        shape -> set_faces(faces);
        return shape;
    }
}
#endif

namespace Vital::Engine {
    // Instantiators //
    Collision_Shape::Collision_Shape() {
        #if defined(VSDK_Client)
        {
            std::lock_guard<std::mutex> lock(mutex);
            buffer.insert(this);
        }
        if (debug_all) set_debug_visible(true);
        #endif
    }

    Collision_Shape::~Collision_Shape() {
        #if defined(VSDK_Client)
        std::lock_guard<std::mutex> lock(mutex);
        buffer.erase(this);
        #endif
    }


    // Hooks //
    void Collision_Shape::_notification(int what) {
        if (what == NOTIFICATION_PREDELETE) Tool::Event::emit("entity:unspawned", Tool::Stack({this}));
    }


    // Managers //
    Collision_Shape* Collision_Shape::create(godot::Node3D* owner) {
        auto body = memnew(Collision_Shape);
        if (owner) owner -> add_child(body);
        else Engine::Core::get_singleton() -> add_child(body);
        Tool::Event::emit("entity:spawned", Tool::Stack({body, false}));
        Tool::Event::emit("entity:ready", Tool::Stack({static_cast<godot::Node3D*>(body)}));
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


    // Checkers //
    bool Collision_Shape::is_replicated() const {
        return get_parent_net_id() > 0;
    }

    #if defined(VSDK_Client)
    bool Collision_Shape::is_debug_visible() const {
        return debug_mesh && debug_mesh -> is_visible();
    }

    bool Collision_Shape::is_debug_all() {
        return debug_all;
    }
    #endif


    // Getters //
    uint32_t Collision_Shape::get_parent_net_id() const {
        auto parent = const_cast<Collision_Shape*>(this) -> get_parent();
        if (!parent) return 0;
        auto syncable = dynamic_cast<ISyncable*>(godot::Object::cast_to<godot::Object>(parent));
        return syncable ? syncable -> get_net_id() : 0;
    }


    // Setters //
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

    void Collision_Shape::set_debug_all(bool state) {
        debug_all = state;
        std::lock_guard<std::mutex> lock(mutex);
        for (auto shape : buffer) shape -> set_debug_visible(state);
    }
    #endif


    // Misc //
    #if defined(VSDK_Client)
    void Collision_Shape::refresh_debug_mesh() {
        if (!debug_mesh || !current_shape.is_valid()) return;
        auto color = is_replicated() ? replicated_debug_color : local_debug_color;
        debug_mesh -> set_mesh(Internal::build_wireframe_mesh(current_shape, color));
    }
    #endif
}
