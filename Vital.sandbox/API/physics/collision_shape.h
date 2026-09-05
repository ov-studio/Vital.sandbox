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
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Manager/public/network.h>
#include <Vital.sandbox/Engine/public/collision_shape.h>
#include <Vital.sandbox/API/physics/rigid_body.h>
#include <Vital.sandbox/API/physics/static_body.h>
#include <Vital.sandbox/API/physics/character_body.h>
#include <Vital.sandbox/API/physics/animatable_body.h>
#include <Vital.sandbox/API/physics/area.h>


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

            auto get_node() {
                return body;
            }

            bool is_alive() const {
                return body ? true : false;
            }

            // Server-side: broadcast shape type + params to all clients via RPC.
            // No-op for local (net_id == 0) bodies. Delegates parent lookup to
            // Engine::Collision_Shape::get_parent_net_id() — no duplicate cast here.
            void broadcast_shape(const char* shape_type, godot::Array params) {
                #if !defined(VSDK_Client)
                uint32_t nid = body ? body->get_parent_net_id() : 0;
                if (nid == 0) return;
                auto net = Manager::Network::get_singleton() -> get_node();
                if (net) net -> rpc("_sync_shape", (int)nid, godot::String(shape_type), params);
                #endif
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> body) {
                    instance -> body -> destroy();
                    instance -> body = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

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
                // Engine::Collision_Shape::create() now applies the current
                // default_debug_enabled itself — nothing else to do here.
                instance -> body = base_class::create(owner);
                instance -> store(true);
                return 1;
            });

            #if defined(VSDK_Client)
            // Global toggle: shows/hides wireframes on every collision shape that currently
            // exists (Lua-created AND network-replicated alike), and sets the default for
            // any collision shape created afterward. Delegates straight to the engine layer,
            // which tracks live shapes independently of the Lua registry.
            API::bind(vm, base_scope, "set_debug_all", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                base_class::set_debug_all(state);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "is_debug_all", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::default_debug_enabled);
                return 1;
            });
            #endif
        }

        static void methods(Machine* vm) {
            API::Node_3D::methods<Instance, Node_3D::Type::Spatial>(vm);
            // set_parent/get_parent intentionally NOT bound here: a Collision_Shape
            // is always created attached to its owning body (Collision_Shape::create()
            // add_child()s it directly — see "create" above) and isn't ISyncable itself,
            // so parent_methods' server/client rules (Rule A/B/C) don't have a net_id
            // to key off for it. Exposing set_parent would let client Lua silently
            // detach a shape from a server-owned body with no restriction at all.

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

            #if defined(VSDK_Client)
            vm_module::bind_method<Instance>(vm, "is_debug_visible", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_debug_visible());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_debug_visible", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_debug_visible(state);
                vm -> push_value(true);
                return 1;
            });
            #endif

            vm_module::bind_method<Instance>(vm, "set_shape_box", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(size)", true)
                    .require(2, &Machine::is_vector3);

                auto size = vm -> get_vector3(2);
                godot::Ref<godot::BoxShape3D> shape;
                shape.instantiate();
                shape -> set_size(size);
                self -> body -> assign_shape(shape);
                #if !defined(VSDK_Client)
                { godot::Array p; p.push_back(size.x); p.push_back(size.y); p.push_back(size.z); self -> broadcast_shape("box", p); }
                #endif
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
                self -> body -> assign_shape(shape);
                #if !defined(VSDK_Client)
                { godot::Array p; p.push_back(radius); self -> broadcast_shape("sphere", p); }
                #endif
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
                self -> body -> assign_shape(shape);
                #if !defined(VSDK_Client)
                { godot::Array p; p.push_back(radius); p.push_back(height); self -> broadcast_shape("capsule", p); }
                #endif
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
                self -> body -> assign_shape(shape);
                #if !defined(VSDK_Client)
                { godot::Array p; p.push_back(radius); p.push_back(height); self -> broadcast_shape("cylinder", p); }
                #endif
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
                self -> body -> assign_shape(shape);
                #if !defined(VSDK_Client)
                { godot::Array p; p.push_back(normal.x); p.push_back(normal.y); p.push_back(normal.z); p.push_back(distance); self -> broadcast_shape("world_boundary", p); }
                #endif
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
                self -> body -> assign_shape(shape);
                #if !defined(VSDK_Client)
                { godot::Array p; p.push_back(length); self -> broadcast_shape("separation_ray", p); }
                #endif
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
}
