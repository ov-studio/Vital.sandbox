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
#include <Vital.sandbox/API/physics/physics_entity.h>
#include <Vital.sandbox/API/physics/area.h>
#include <Vital.sandbox/API/core/model.h>


//////////////////////////////////
// Vital: API: Collision_Shape //
//////////////////////////////////

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

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> body) {
                    instance -> body -> destroy();
                    instance -> body = nullptr;
                }
                instance -> release();
            }

            void broadcast(const char* shape_type, godot::Array params) {
                #if !defined(VSDK_Client)
                uint32_t nid = body ? body -> get_parent_net_id() : 0;
                if (nid == 0) {
                    if (body) {
                        auto parent = body -> get_parent();
                        if (parent) {
                            using PB_Rigid       = Vital::Engine::Physics_Body<godot::RigidBody3D>;
                            using PB_Static      = Vital::Engine::Physics_Body<godot::StaticBody3D>;
                            using PB_Character   = Vital::Engine::Physics_Body<godot::CharacterBody3D>;
                            using PB_Animatable  = Vital::Engine::Physics_Body<godot::AnimatableBody3D>;
                            auto try_stash = [&](auto pb) -> bool {
                                if (!pb) return false;
                                pb -> pending_shape_broadcast = { shape_type, params };
                                return true;
                            };
                            try_stash(dynamic_cast<PB_Rigid*>(parent))      ||
                            try_stash(dynamic_cast<PB_Static*>(parent))     ||
                            try_stash(dynamic_cast<PB_Character*>(parent))  ||
                            try_stash(dynamic_cast<PB_Animatable*>(parent));
                        }
                    }
                    return;
                }

                auto net = Manager::Network::get_singleton() -> get_node();
                if (net) net -> rpc("_sync_shape", (int)nid, godot::String(shape_type), params);
                #endif
            }
        };
        inline static vm_registry<Instance> registry;

        static godot::Node3D* resolve_owner(Machine* vm, int idx) {
            if (auto node = Physics_Entity::resolve_body(vm, idx)) return node;
            if (vm_module::is_userdata<Area::Instance>(vm, idx)) return vm_module::get_userdata_object<Area::Instance>(vm, idx) -> get_node();
            return nullptr;
        }

        static void init(Machine* vm) {
            static Tool::Event::Handle spawned_binding;
            spawned_binding.bind("entity:spawned", [](Tool::Stack args) {
                if (args.array.size() < 2) return;
                if (!args.array[0].is_raw_ptr<base_class>()) return;
                auto entity = args.array[0].as_raw_ptr<base_class>();
                if (!args.array[1].is<bool>()) return;
                bool remote = args.array[1].as<bool>();
                if (Instance::find_by_ptr(entity)) return;

                const godot::ObjectID oid(entity -> get_instance_id());
                Vital::Engine::Core::get_singleton() -> enqueue([oid, remote]() {
                    godot::Object* obj = godot::ObjectDB::get_instance(oid);
                    if (!obj) return;
                    auto shape = godot::Object::cast_to<base_class>(obj);
                    if (!shape) return;
                    if (Instance::find_by_ptr(shape)) return;
                    
                    auto instance = Instance::init(nullptr, remote);
                    instance -> body = shape;
                    instance -> store(false);
                });
            });

            static Tool::Event::Handle unspawned_binding;
            unspawned_binding.bind("entity:unspawned", [](Tool::Stack args) {
                if (args.array.size() < 1) return;
                if (!args.array[0].is_raw_ptr<base_class>()) return;
                
                auto entity = args.array[0].as_raw_ptr<base_class>();
                Instance::destroy_by_ptr(entity, [](std::shared_ptr<Instance> instance) {
                    instance -> body = nullptr;
                });
            });
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Collision_Shape>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(body)", true)
                    .require(1, [](Machine* vm, int idx) { return resolve_owner(vm, idx) != nullptr; });

                auto body = resolve_owner(vm, 1);
                auto instance = Instance::init(vm);
                instance -> body = base_class::create(body);
                instance -> store(true);
                return 1;
            });

            #if defined(VSDK_Client)
            API::bind(vm, base_scope, "is_debug_all", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::is_debug_all());
                return 1;
            });

            API::bind(vm, base_scope, "set_debug_all", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(1, &Machine::is_bool);

                auto state = vm -> get_bool(1);
                base_class::set_debug_all(state);
                vm -> push_value(true);
                return 1;
            });
            #endif
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
                { 
                    godot::Array p; 
                    p.push_back(size.x); 
                    p.push_back(size.y); 
                    p.push_back(size.z); 
                    self -> broadcast("box", p); 
                }
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
                { 
                    godot::Array p; 
                    p.push_back(radius); 
                    self -> broadcast("sphere", p); 
                }
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
                { 
                    godot::Array p; p.push_back(radius); 
                    p.push_back(height); 
                    self -> broadcast("capsule", p); 
                }
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
                { 
                    godot::Array p; 
                    p.push_back(radius); 
                    p.push_back(height); 
                    self -> broadcast("cylinder", p); 
                }
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
                { 
                    godot::Array p; 
                    p.push_back(normal.x); 
                    p.push_back(normal.y); 
                    p.push_back(normal.z); 
                    p.push_back(distance); 
                    self -> broadcast("world_boundary", p); 
                }
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
                { 
                    godot::Array p; 
                    p.push_back(length); 
                    self -> broadcast("separation_ray", p); 
                }
                #endif
                vm -> push_value(true);
                return 1;
            });


            /*
             * set_shape_mesh(model, config = {})
             *
             * config (all optional):
             *   shape_type       = "convex"|"concave"  -- default "convex"
             *                                             used as fallback when no filter matches,
             *                                             and as the sole type when include_children=false
             *   include_children = bool                -- default false
             *                                             true: one child CollisionShape3D per mesh,
             *                                             parented to self->body at the mesh's local offset
             *   filters = {                            -- ORDERED array-of-tables, first-match-wins.
             *                                             Lua hash tables have no guaranteed iteration
             *                                             order, so filters MUST use the array form:
             *     { match = "*_leaves_*",   shape_type = "none"    },  -- checked first (highest priority)
             *     { match = "*_interior_*", shape_type = "concave" },
             *     { match = "*_terrain_*",  shape_type = "convex"  },
             *     { match = "*",            shape_type = "convex"  },  -- catch-all (lowest priority)
             *   }
             *   Wildcards: "*" matches any substring inside a part name.
             *   shape_type = "none" skips the mesh entirely (no CollisionShape3D created for it).
             *
             * Sync behaviour:
             *   Server broadcasts "mesh_ref" with all per-child shape data packed into params.
             *   Clients reconstruct the full set of child CollisionShape3D nodes from that single RPC.
             *   Late-joiners are covered by the existing join-sync path in Manager/private/network.cpp
             *   which was extended to serialise ConvexPolygonShape3D and ConcavePolygonShape3D children.
             */
             // TODO: Improve
            vm_module::bind_method<Instance>(vm, "set_shape_mesh", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(model, config = {})", true)
                    .require(2, [](Machine* vm, int idx) { return vm_module::is_userdata<Vital::Sandbox::API::Model::Instance>(vm, idx); })
                    .optional(3, &Machine::is_table);

                auto model = vm_module::get_userdata_object<Vital::Sandbox::API::Model::Instance>(vm, 2);

                // Parse config //
                std::string shape_type = "convex";
                bool include_children = false;
                std::vector<std::pair<std::string, std::string>> filters;
                if (vm -> is_table(3)) {
                    vm -> get_table_field("shape_type", 3);
                    if (vm -> is_string(-1)) shape_type = vm -> get_string(-1);
                    vm -> pop(1);
                    vm -> get_table_field("include_children", 3);
                    if (vm -> is_bool(-1)) include_children = vm -> get_bool(-1);
                    vm -> pop(1);
                    vm -> get_table_field("filters", 3);
                    if (vm -> is_table(-1)) {
                        int n = vm -> get_length(-1);
                        for (int i = 1; i <= n; ++i) {
                            vm -> get_table_field(i, -1);
                            if (vm -> is_table(-1)) {
                                vm -> get_table_field("match", -1);
                                std::string match_str = vm -> is_string(-1) ? vm -> get_string(-1) : "";
                                vm -> pop(1);
                                vm -> get_table_field("shape_type", -1);
                                std::string type_str = vm -> is_string(-1) ? vm -> get_string(-1) : "";
                                vm -> pop(1);
                                if (!match_str.empty() && !type_str.empty()) filters.emplace_back(match_str, type_str);
                            }
                            vm -> pop(1);
                        }
                    }
                    vm -> pop(1); 
                }

                auto* model_node = model -> get_node();
                auto resolve_type = [&](const std::string& component) -> std::string {
                    if (filters.empty()) return shape_type;
                    auto slash = component.rfind('/');
                    auto leaf  = (slash == std::string::npos) ? component : component.substr(slash + 1);
                    for (auto& [pattern, type] : filters) {
                        if (Tool::match_wildcard(pattern, leaf) || Tool::match_wildcard(pattern, component)) return type;
                    }
                    return shape_type;
                };

                // Build a shape ref from a MeshInstance3D* given type string.
                auto build_shape = [&](godot::MeshInstance3D* mesh, const std::string& type) -> godot::Ref<godot::Shape3D> {
                    if (type == "concave") return base_class::Internal::build_concave_shape(mesh);
                    return base_class::Internal::build_convex_shape(mesh);
                };

                if (!include_children) {
                    // Single mesh: use root if it's a MeshInstance3D, else the first
                    // MeshInstance3D found anywhere in the model's subtree (glTF imports
                    // routinely nest the mesh 2+ levels below the scene root, so a
                    // shallow immediate-children scan misses it entirely).
                    godot::MeshInstance3D* mesh = godot::Object::cast_to<godot::MeshInstance3D>(model_node);
                    std::string used_component; // empty == "the model root itself"
                    if (!mesh) {
                        auto components = model_node -> get_components();
                        if (!components.empty()) {
                            mesh = model_node -> find_mesh_node(model_node, components[0]);
                            used_component = components[0];
                        }
                    }
                    if (!mesh) { vm -> push_value(false); return 1; }
                    auto shape = build_shape(mesh, shape_type);
                    if (!shape.is_valid()) { vm -> push_value(false); return 1; }
                    self -> body -> assign_shape(shape);
                    #if !defined(VSDK_Client)
                    {
                        godot::Array p;
                        p.push_back((int)model_node -> get_net_id());
                        p.push_back((int)0); // mode 0: single mesh — shape goes directly on the sync target (`col`), no anchor/children involved.
                        p.push_back(godot::String(used_component.c_str()));
                        p.push_back(godot::String(shape_type == "concave" ? "concave" : "convex"));
                        p.push_back(0.f); p.push_back(0.f); p.push_back(0.f); // pos (identity — body origin)
                        p.push_back(0.f); p.push_back(0.f); p.push_back(0.f); // rot (euler deg)
                        self -> broadcast("mesh_ref", p);
                    }
                    #endif
                }
                else {
                    // Multi-mesh mode.
                    auto components = model_node -> get_components();
                    if (components.empty()) { vm -> push_value(false); return 1; }
    
                    // Anchor shape so the root CollisionShape3D is a valid node (zero-size box).
                    {
                        godot::Ref<godot::BoxShape3D> anchor;
                        anchor.instantiate();
                        anchor -> set_size(godot::Vector3(0.001f, 0.001f, 0.001f));
                        self -> body -> assign_shape(anchor);
                    }
    
                    // Sync by REFERENCE (mesh_ref), same rationale as the single-mesh
                    // branch above: one model net_id + one (component, type, transform)
                    // tuple per child, no vertex/face data. Clients rebuild each child
                    // shape locally from their own already-loaded copy of the model.
                    #if !defined(VSDK_Client)
                    godot::Array sync_params;
                    sync_params.push_back((int)model_node -> get_net_id());
                    sync_params.push_back((int)1); // mode 1: include_children — `col` stays the (untouched) anchor, every matched mesh becomes its own child.
                    #endif
    
                    for (auto& component : components) {
                        auto resolved = resolve_type(component);
                        if (resolved == "none") continue;

                        auto* mesh = model_node -> find_mesh_node(model_node, component);
                        if (!mesh) continue;
                        auto shape = build_shape(mesh, resolved);
                        if (!shape.is_valid()) continue;
    
                        // Parent each per-mesh Collision_Shape DIRECTLY under the physics body
                        // (StaticBody / RigidBody / …), not under the intermediate anchor
                        // Collision_Shape. Nested CollisionShape3D under another
                        // CollisionShape3D often fails to register with the shape body —
                        // debug wireframe is visible but the character never collides.
                        godot::Node3D* physics_body = godot::Object::cast_to<godot::Node3D>(self -> body -> get_parent());
                        if (!physics_body) physics_body = self -> body; // fallback
    
                        // Offset: mesh transform relative to the PHYSICS BODY in world
                        // space. Model-root relative was fragile (only matched when the
                        // model sat unparented at the same world pose as the body).
                        // Body-relative is correct whether the model is already parented
                        // under the body or still at world origin, as long as body and
                        // model share the same world pose when set_shape_mesh runs.
                        auto rel = physics_body -> get_global_transform().inverse() * mesh -> get_global_transform();
                        auto* child = base_class::create(physics_body);
                        // Ensure the shape is registered with the body's shape body.
                        // set_shape while already in-tree, force enabled, apply local
                        // transform, then toggle disabled to force a shape-body refresh
                        // (Godot can miss registration when shape + transform are set
                        // in the same tick as add_child).
                        child -> set_disabled(false);
                        child -> assign_shape(shape);
                        child -> set_transform(rel);
                        child -> set_disabled(true);
                        child -> set_disabled(false);

                        #if !defined(VSDK_Client)
                        sync_params.push_back(godot::String(component.c_str()));
                        sync_params.push_back(godot::String(resolved == "concave" ? "concave" : "convex"));
                        sync_params.push_back(rel.origin.x);
                        sync_params.push_back(rel.origin.y);
                        sync_params.push_back(rel.origin.z);
                        auto euler = rel.basis.get_euler() * (180.f / 3.14159265358979323846f);
                        sync_params.push_back(euler.x);
                        sync_params.push_back(euler.y);
                        sync_params.push_back(euler.z);
                        #endif
                    }
    
                    #if !defined(VSDK_Client)
                    // Only the net_id + mode ints are present when nothing matched — nothing to send.
                    if (sync_params.size() > 2) self -> broadcast("mesh_ref", sync_params);
                    #endif
                }
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
