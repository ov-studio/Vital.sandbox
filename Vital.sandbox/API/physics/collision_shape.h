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

        // TODO: Reuse Tool/index.h
        // Wildcard pattern matching: supports * anywhere in pattern //
        static bool match_wildcard(const std::string& pattern, const std::string& name) {
            if (pattern == "*") return true;
            std::size_t pi = 0, ni = 0, star_pi = std::string::npos, star_ni = 0;
            while (ni < name.size()) {
                if (pi < pattern.size() && (pattern[pi] == name[ni] || pattern[pi] == '?')) {
                    ++pi; ++ni;
                } else if (pi < pattern.size() && pattern[pi] == '*') {
                    star_pi = pi++;
                    star_ni = ni;
                } else if (star_pi != std::string::npos) {
                    pi = star_pi + 1;
                    ni = ++star_ni;
                } else return false;
            }
            while (pi < pattern.size() && pattern[pi] == '*') ++pi;
            return pi == pattern.size();
        }

        // Resolve filter map from Lua table at stack index idx into a std::vector of {pattern, type} pairs //
        // Returns false if the table is missing or malformed. type is "convex", "concave", or "none".      //
        static bool resolve_filters(Machine* vm, int idx, std::vector<std::pair<std::string, std::string>>& out) {
            if (!vm -> is_table(idx)) return false;
            lua_State* L = vm -> get_state();
            lua_pushnil(L);
            while (lua_next(L, idx < 0 ? idx - 1 : idx) != 0) {
                if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TSTRING) {
                    out.emplace_back(lua_tostring(L, -2), lua_tostring(L, -1));
                }
                lua_pop(L, 1);
            }
            return true;
        }

        // Match a mesh component name against the ordered filter list, return matched type or empty string //
        static std::string apply_filters(const std::string& component, const std::vector<std::pair<std::string, std::string>>& filters) {
            for (auto& [pattern, type] : filters) {
                // Extract just the leaf name for matching (after the last /) //
                auto slash = component.rfind('/');
                auto leaf  = (slash == std::string::npos) ? component : component.substr(slash + 1);
                if (match_wildcard(pattern, leaf) || match_wildcard(pattern, component))
                    return type;
            }
            return "";
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
                vm_args(vm, id, "(owner)", true)
                    .require(1, [](Machine* vm, int idx) { return resolve_owner(vm, idx) != nullptr; });

                auto owner = resolve_owner(vm, 1);
                auto instance = Instance::init(vm);
                instance -> body = base_class::create(owner);
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


            // TODO: WIP
            /*
             * set_shape_mesh(model, config = {})
             *
             * config fields (all optional):
             *   shape_type        = "convex"|"concave"   -- default "convex", used when include_children=false
             *                                               or as the catch-all when no filter matches
             *   include_children  = bool                 -- default false; when true, creates one child
             *                                               CollisionShape3D per matching MeshInstance3D in
             *                                               the model and parents them to this shape's Node3D
             *                                               at the mesh's local offset
             *   filters           = {                    -- ordered list of {pattern, type} pairs
             *     ["*_terrain_*"] = "convex",            --   wildcard matched against leaf mesh name
             *     ["*_interior_*"] = "concave",          --   first match wins
             *     ["*_leaves_*"]  = "none",              --   "none" skips that mesh
             *     ["*"]           = "convex",            --   catch-all fallback
             *   }
             *
             * When include_children=false: builds one shape from the root MeshInstance3D (or first found)
             *   using shape_type, and calls assign_shape on self.
             * When include_children=true: iterates all MeshInstance3D children via model->get_components(),
             *   resolves each against filters (falling back to shape_type), skips "none", and attaches
             *   a child Collision_Shape parented to self->body at the mesh's relative transform.
             */
            vm_module::bind_method<Instance>(vm, "set_shape_mesh", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(model, config = {})", true)
                    .require(2, [](Machine* vm, int idx) {
                        return vm_module::is_userdata<Vital::Sandbox::API::Model::Instance>(vm, idx);
                    })
                    .optional(3, &Machine::is_table);

                auto model_inst = vm_module::get_userdata_object<Vital::Sandbox::API::Model::Instance>(vm, 2);
                if (!model_inst || !model_inst -> model) { vm -> push_value(false); return 1; }

                // Parse config //
                std::string shape_type      = "convex";
                bool include_children       = false;
                std::vector<std::pair<std::string, std::string>> filters;

                if (vm -> is_table(3)) {
                    lua_State* L = vm -> get_state();

                    lua_getfield(L, 3, "shape_type");
                    if (lua_type(L, -1) == LUA_TSTRING) shape_type = lua_tostring(L, -1);
                    lua_pop(L, 1);

                    lua_getfield(L, 3, "include_children");
                    if (lua_type(L, -1) == LUA_TBOOLEAN) include_children = lua_toboolean(L, -1);
                    lua_pop(L, 1);

                    lua_getfield(L, 3, "filters");
                    if (lua_type(L, -1) == LUA_TTABLE) {
                        lua_pushnil(L);
                        while (lua_next(L, -2) != 0) {
                            if (lua_type(L, -2) == LUA_TSTRING && lua_type(L, -1) == LUA_TSTRING)
                                filters.emplace_back(lua_tostring(L, -2), lua_tostring(L, -1));
                            lua_pop(L, 1);
                        }
                    }
                    lua_pop(L, 1);
                }

                auto* model_node = model_inst -> model;

                // Helper: resolve the correct MeshInstance3D* from a component path string //
                auto get_mesh = [&](const std::string& component) -> godot::MeshInstance3D* {
                    return model_node -> find_mesh_node(model_node, component);
                };

                // Helper: build a shape from a MeshInstance3D* given a type string //
                auto build_shape = [&](godot::MeshInstance3D* mesh, const std::string& type)
                    -> godot::Ref<godot::Shape3D>
                {
                    if (type == "concave") return base_class::Internal::build_concave_shape(mesh);
                    return base_class::Internal::build_convex_shape(mesh); // default convex
                };

                if (!include_children) {
                    // Single-mesh mode: use root or first MeshInstance3D child //
                    godot::MeshInstance3D* mesh = godot::Object::cast_to<godot::MeshInstance3D>(model_node);
                    if (!mesh) {
                        for (int i = 0; i < model_node -> get_child_count(); i++) {
                            mesh = godot::Object::cast_to<godot::MeshInstance3D>(model_node -> get_child(i));
                            if (mesh) break;
                        }
                    }
                    if (!mesh) { vm -> push_value(false); return 1; }
                    auto shape = build_shape(mesh, shape_type);
                    if (!shape.is_valid()) { vm -> push_value(false); return 1; }
                    self -> body -> assign_shape(shape);
                    vm -> push_value(true);
                    return 1;
                }

                // Multi-mesh mode: iterate all components, apply filters, attach child shapes //
                auto components = model_node -> get_components();
                if (components.empty()) { vm -> push_value(false); return 1; }

                // Give the root shape a dummy shape so it acts as a valid anchor node //
                // (CollisionShape3D with no shape causes Godot warnings)
                {
                    godot::Ref<godot::BoxShape3D> anchor;
                    anchor.instantiate();
                    anchor -> set_size(godot::Vector3(0.001f, 0.001f, 0.001f));
                    self -> body -> assign_shape(anchor);
                }

                for (auto& component : components) {
                    // Resolve filter type for this component //
                    std::string resolved_type = filters.empty() ? shape_type : "";
                    if (!filters.empty()) {
                        // Extract leaf name (after last /) for matching //
                        auto slash = component.rfind('/');
                        auto leaf  = (slash == std::string::npos) ? component : component.substr(slash + 1);
                        for (auto& [pattern, type] : filters) {
                            if (match_wildcard(pattern, leaf) || match_wildcard(pattern, component)) {
                                resolved_type = type;
                                break;
                            }
                        }
                        // If no filter matched, fall back to shape_type //
                        if (resolved_type.empty()) resolved_type = shape_type;
                    }
                    if (resolved_type == "none") continue;

                    auto* mesh = get_mesh(component);
                    if (!mesh) continue;
                    auto shape = build_shape(mesh, resolved_type);
                    if (!shape.is_valid()) continue;

                    // Create child CollisionShape3D parented to self->body, offset to mesh position //
                    auto* child = base_class::create(self -> body);
                    child -> assign_shape(shape);
                    // Apply the mesh's transform relative to the model root so the child
                    // sits exactly where the mesh is in world space when the parent moves //
                    child -> set_transform(model_node -> get_transform().inverse() * mesh -> get_global_transform());
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
