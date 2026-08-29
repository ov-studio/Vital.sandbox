/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: physics: collision_shape.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Collision Shape APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/collision_shape.h>
#include <Vital.sandbox/API/physics/rigid_body.h>
#include <Vital.sandbox/API/physics/static_body.h>
#include <Vital.sandbox/API/physics/character_body.h>
#include <Vital.sandbox/API/physics/animatable_body.h>
#include <Vital.sandbox/API/physics/area.h>
#include <godot_cpp/classes/box_shape3d.hpp>
#include <godot_cpp/classes/sphere_shape3d.hpp>
#include <godot_cpp/classes/capsule_shape3d.hpp>
#include <godot_cpp/classes/cylinder_shape3d.hpp>
#include <godot_cpp/classes/world_boundary_shape3d.hpp>
#include <godot_cpp/classes/separation_ray_shape3d.hpp>


//////////////////////////////////
// Vital: API: Collision_Shape //
//////////////////////////////////

// TODO: Improve
namespace Vital::Sandbox::API {
    struct Collision_Shape : vm_module {
        inline static const std::vector<std::string> base_scope = {"physics", "collision_shape"};
        using base_class = Vital::Engine::Collision_Shape;

        struct Instance : vm_instance<Instance> {
            using Owner = Collision_Shape;
            base_class* body = nullptr;
            godot::Ref<godot::Shape3D> current_shape;
            godot::MeshInstance3D* debug_mesh = nullptr;

            auto get_node() {
                return body;
            }

            bool is_alive() const {
                return body ? true : false;
            }

            // Builds/rebuilds the wireframe debug mesh from whatever shape is currently assigned. //
            void refresh_debug_mesh();

            void set_debug_visible(bool state) {
                if (!body) return;
                if (state) {
                    if (!debug_mesh) {
                        debug_mesh = memnew(godot::MeshInstance3D);
                        body -> add_child(debug_mesh);
                    }
                    debug_mesh -> set_visible(true);
                    refresh_debug_mesh();
                }
                else if (debug_mesh) debug_mesh -> set_visible(false);
            }

            bool is_debug_visible() const {
                return debug_mesh && debug_mesh -> is_visible();
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> debug_mesh) {
                    instance -> debug_mesh -> queue_free();
                    instance -> debug_mesh = nullptr;
                }
                if (instance -> body) {
                    instance -> body -> destroy();
                    instance -> body = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;
        inline static bool default_debug_enabled = false;

        // Wireframe geometry builders — one closed ring/line-set per shape type, assembled into a PRIMITIVE_LINES mesh. //
        static void add_ring(godot::PackedVector3Array& points, float radius, float y, int plane, int segments = 24) {
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

        static void add_half_ring(godot::PackedVector3Array& points, float radius, float center_y, int plane, bool upper, int segments = 12) {
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

        static godot::Ref<godot::ArrayMesh> build_wireframe_mesh(const godot::Ref<godot::Shape3D>& shape) {
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
                material -> set_albedo(godot::Color(0, 1, 0));
                mesh -> surface_set_material(0, material);
            }
            return mesh;
        }

        // Resolves any of the physics body/area API types to their underlying Node3D owner. //
        static godot::Node3D* resolve_owner(Machine* vm, int idx) {
            if (vm_module::is_userdata<Rigid_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Rigid_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Static_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Static_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Character_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Character_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Animatable_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Animatable_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Area::Instance>(vm, idx)) return vm_module::get_userdata_object<Area::Instance>(vm, idx) -> get_node();
            return nullptr;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Collision_Shape>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(owner)", true)
                    .require(1, [](Machine* vm, int idx) { return resolve_owner(vm, idx) != nullptr; });

                auto owner = resolve_owner(vm, 1);
                auto instance = Instance::init(vm);
                instance -> body = base_class::create(owner);
                if (default_debug_enabled) instance -> set_debug_visible(true);
                instance -> store(true);
                return 1;
            });

            // Global toggle: shows/hides wireframes on every collision shape that currently exists,
            // and sets the default for any collision shape created afterward. //
            API::bind(vm, base_scope, "set_debug_all", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                default_debug_enabled = state;
                std::lock_guard<std::mutex> lock(registry.mutex);
                for (auto& [key, instance] : registry.buffer) {
                    if (Instance::find_unlocked(instance)) instance -> set_debug_visible(state);
                }
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "is_debug_all", [](auto vm, auto& id) -> int {
                vm -> push_value(default_debug_enabled);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Node_3D::methods<Instance, Node_3D::Type::Spatial>(vm);

            vm_module::bind_method<Instance>(vm, "is_disabled", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_disabled());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_disabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_disabled(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_debug_visible", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> is_debug_visible());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_debug_visible", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> set_debug_visible(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shape_box", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(size)", true)
                    .require(2, &Machine::is_vector3);

                auto size = vm -> get_vector3(2);
                godot::Ref<godot::BoxShape3D> shape;
                shape.instantiate();
                shape -> set_size(size);
                self -> body -> set_shape(shape);
                self -> current_shape = shape;
                self -> refresh_debug_mesh();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shape_sphere", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(radius)", true)
                    .require(2, &Machine::is_number);

                auto radius = vm -> get_float(2);
                godot::Ref<godot::SphereShape3D> shape;
                shape.instantiate();
                shape -> set_radius(radius);
                self -> body -> set_shape(shape);
                self -> current_shape = shape;
                self -> refresh_debug_mesh();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shape_capsule", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(radius, height)", true)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_number);

                auto radius = vm -> get_float(2);
                auto height = vm -> get_float(3);
                godot::Ref<godot::CapsuleShape3D> shape;
                shape.instantiate();
                shape -> set_radius(radius);
                shape -> set_height(height);
                self -> body -> set_shape(shape);
                self -> current_shape = shape;
                self -> refresh_debug_mesh();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shape_cylinder", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(radius, height)", true)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_number);

                auto radius = vm -> get_float(2);
                auto height = vm -> get_float(3);
                godot::Ref<godot::CylinderShape3D> shape;
                shape.instantiate();
                shape -> set_radius(radius);
                shape -> set_height(height);
                self -> body -> set_shape(shape);
                self -> current_shape = shape;
                self -> refresh_debug_mesh();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shape_world_boundary", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(plane_normal, plane_distance = 0)", true)
                    .require(2, &Machine::is_vector3)
                    .optional(3, &Machine::is_number);

                auto normal = vm -> get_vector3(2);
                auto distance = vm -> is_number(3) ? vm -> get_float(3) : 0.f;
                godot::Ref<godot::WorldBoundaryShape3D> shape;
                shape.instantiate();
                shape -> set_plane(godot::Plane(normal, distance));
                self -> body -> set_shape(shape);
                self -> current_shape = shape;
                self -> refresh_debug_mesh();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shape_separation_ray", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(length)", true)
                    .require(2, &Machine::is_number);

                auto length = vm -> get_float(2);
                godot::Ref<godot::SeparationRayShape3D> shape;
                shape.instantiate();
                shape -> set_length(length);
                self -> body -> set_shape(shape);
                self -> current_shape = shape;
                self -> refresh_debug_mesh();
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            API::Node_3D::inject<Instance>(vm);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };

    inline void Collision_Shape::Instance::refresh_debug_mesh() {
        if (!debug_mesh || !current_shape.is_valid()) return;
        debug_mesh -> set_mesh(Collision_Shape::build_wireframe_mesh(current_shape));
    }
}
#else
namespace Vital::Sandbox::API {
    struct Collision_Shape : vm_module {};
}
#endif
