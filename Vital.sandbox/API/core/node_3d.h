/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: node_3d.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Node 3D APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>


//////////////////////////
// Vital: API: Node 3D //
//////////////////////////

namespace Vital::Sandbox {
    struct vm_instance_base;
    extern std::unordered_map<void*, vm_instance_base*> vm_node_registry;
    extern std::mutex vm_node_registry_mutex;
}

namespace Vital::Sandbox::API {
    struct Node_3D {
        enum class Type {
            Camera,
            Audio,
            Spatial
        };

        template<typename Instance, Type node_type = Type::Spatial>
        static void bind(Machine* vm) {}

        template<typename Instance, Type node_type = Type::Spatial>
        static void methods(Machine* vm) {
            if constexpr (node_type == Type::Spatial) {
                vm_module::bind_method<Instance>(vm, "is_visible", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> is_visible());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "is_visible_in_tree", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> is_visible_in_tree());
                    return 1;
                });

            }
            {
                vm_module::bind_method<Instance>(vm, "get_position", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> get_position());
                    return 1;
                });
    
                vm_module::bind_method<Instance>(vm, "get_global_position", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> get_global_position());
                    return 1;
                });
            }
            if constexpr (node_type == Type::Spatial || node_type == Type::Camera) {
                vm_module::bind_method<Instance>(vm, "get_scale", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> get_scale());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_global_scale", [](auto vm, auto self, auto& id) -> int {
                    auto scale = self -> get_node() -> get_global_transform().basis.get_scale();
                    vm -> push_value(scale);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_rotation", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> get_rotation_degrees());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_global_rotation", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> get_node() -> get_global_rotation_degrees());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_quaternion", [](auto vm, auto self, auto& id) -> int {
                    auto quaternion = self -> get_node() -> get_quaternion();
                    auto value = godot::Vector4(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
                    vm -> push_value(value);
                    return 1;
                });
            }
            {
                vm_module::bind_method<Instance>(vm, "set_position", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(position)", true)
                        .require(2, &Machine::is_vector3);
    
                    auto position = vm -> get_vector3(2);
                    self -> get_node() -> set_position(position);
                    vm -> push_value(true);
                    return 1;
                });
    
                vm_module::bind_method<Instance>(vm, "set_global_position", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(position)", true)
                        .require(2, &Machine::is_vector3);
    
                    auto position = vm -> get_vector3(2);
                    self -> get_node() -> set_global_position(position);
                    vm -> push_value(true);
                    return 1;
                });
    
            }
            if constexpr (node_type == Type::Spatial || node_type == Type::Camera) {
                vm_module::bind_method<Instance>(vm, "set_scale", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(scale)", true)
                        .require(2, &Machine::is_vector3);

                    auto scale = vm -> get_vector3(2);
                    self -> get_node() -> set_scale(scale);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_rotation", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(euler_degrees)", true)
                        .require(2, &Machine::is_vector3);

                    auto euler_degrees = vm -> get_vector3(2);
                    self -> get_node() -> set_rotation_degrees(euler_degrees);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_global_rotation", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(euler_degrees)", true)
                        .require(2, &Machine::is_vector3);

                    auto euler_degrees = vm -> get_vector3(2);
                    self -> get_node() -> set_global_rotation_degrees(euler_degrees);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_quaternion", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(quaternion)", true)
                        .require(2, &Machine::is_vector4);

                    auto value = vm -> get_vector4(2);
                    auto quaternion = godot::Quaternion(value.x, value.y, value.z, value.w);
                    self -> get_node() -> set_quaternion(quaternion);
                    vm -> push_value(true);
                    return 1;
                });
            }
            if constexpr (node_type == Type::Spatial) {
                vm_module::bind_method<Instance>(vm, "set_visible", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(visible)", true)
                        .require(2, &Machine::is_bool);

                    auto visible = vm -> get_bool(2);
                    self -> get_node() -> set_visible(visible);
                    vm -> push_value(true);
                    return 1;
                });
            }
            {
                vm_module::bind_method<Instance>(vm, "translate", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(offset)", true)
                        .require(2, &Machine::is_vector3);
    
                    auto offset = vm -> get_vector3(2);
                    self -> get_node() -> translate(offset);
                    vm -> push_value(true);
                    return 1;
                });
    
                vm_module::bind_method<Instance>(vm, "translate_local", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(offset)", true)
                        .require(2, &Machine::is_vector3);
    
                    auto offset = vm -> get_vector3(2);
                    self -> get_node() -> translate_object_local(offset);
                    vm -> push_value(true);
                    return 1;
                });
            }
            if constexpr (node_type == Type::Spatial || node_type == Type::Camera) {
                vm_module::bind_method<Instance>(vm, "to_local", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(global_point)", true)
                        .require(2, &Machine::is_vector3);

                    auto global_point = vm -> get_vector3(2);
                    auto local_point = self -> get_node() -> to_local(global_point);
                    vm -> push_value(local_point);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "to_global", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(local_point)", true)
                        .require(2, &Machine::is_vector3);

                    auto local_point = vm -> get_vector3(2);
                    auto global_point = self -> get_node() -> to_global(local_point);
                    vm -> push_value(global_point);
                    return 1;
                });
                
                vm_module::bind_method<Instance>(vm, "scale_local", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(scale)", true)
                        .require(2, &Machine::is_vector3);

                    auto scale = vm -> get_vector3(2);
                    self -> get_node() -> scale_object_local(scale);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "scale_global", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(scale)", true)
                        .require(2, &Machine::is_vector3);

                    auto scale = vm -> get_vector3(2);
                    self -> get_node() -> global_scale(scale);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "rotate", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(axis, degrees)", true)
                        .require(2, &Machine::is_vector3)
                        .require(3, &Machine::is_number);

                    auto axis = vm -> get_vector3(2);
                    auto degrees = vm -> get_float(3);
                    self -> get_node() -> rotate(axis, godot::Math::deg_to_rad(degrees));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "rotate_local", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(axis, degrees)", true)
                        .require(2, &Machine::is_vector3)
                        .require(3, &Machine::is_number);

                    auto axis = vm -> get_vector3(2);
                    auto degrees = vm -> get_float(3);
                    self -> get_node() -> rotate_object_local(axis, godot::Math::deg_to_rad(degrees));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "look_at", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(target, up = {0, 1, 0})", true)
                        .require(2, &Machine::is_vector3)
                        .optional(3, &Machine::is_vector3);

                    auto target = vm -> get_vector3(2);
                    auto up = vm -> is_vector3(3) ? vm -> get_vector3(3) : godot::Vector3(0, 1, 0);
                    self -> get_node() -> look_at(target, up);
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "look_at_from_position", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(position, target, up = {0, 1, 0})", true)
                        .require(2, &Machine::is_vector3)
                        .require(3, &Machine::is_vector3)
                        .optional(4, &Machine::is_vector3);

                    auto position = vm -> get_vector3(2);
                    auto target = vm -> get_vector3(3);
                    auto up = vm -> is_vector3(4) ? vm -> get_vector3(4) : godot::Vector3(0, 1, 0);
                    self -> get_node() -> look_at_from_position(position, target, up);
                    vm -> push_value(true);
                    return 1;
                });
            }
        }

        // node_type is accepted for call-site consistency with methods<> but
        // parenting behaviour is identical regardless of node type, so it is
        // intentionally unused inside the body.
        //
        // Ownership rules enforced here:
        //
        //   A) self is a SERVER entity (is_remote() == false, has a net_id > 0):
        //      → May only be parented to another SERVER entity.
        //      → Must only be called from server-side Lua (VSDK_Server compile unit).
        //        The API::Model::methods() block wraps set_parent in #if !VSDK_Client
        //        for server entities, so client Lua never sees this binding on them.
        //      → Internally calls Engine::Model::set_parent() which broadcasts
        //        _reparent_entity to all clients.
        //      → Trying to parent to a client-local entity throws a Lua error.
        //
        //   B) self is a CLIENT entity (client-only, no net_id):
        //      → May be parented to any other entity — including a server entity
        //        (e.g. camera parented to a remote model).  The reparent is purely
        //        local; no RPC is sent.
        //      → May NOT be used to parent a server entity (covered by rule A).
        //      → Trying to parent a client entity to another client entity that is
        //        itself a server entity's child is fine (Godot handles it locally).
        //
        //   C) Client Lua MUST NOT call set_parent on a server entity.
        //      The #if !defined(VSDK_Client) guard in model.h::methods() ensures
        //      the binding simply does not exist in the client VM for server-owned
        //      models; any attempt produces a nil-method error naturally.

        // Thin wrapper around Core::when_parent_ready() for the raw (non-Model)
        // reparent calls below — physics bodies today, any future node type
        // that funnels through this generic binding. The actual defer/retry
        // logic lives once, in Engine::Core, shared with Model::set_parent().
        static void reparent_safe(Vital::Engine::Core* core, godot::Node3D* node, godot::Node* target) {
            if (!core || !node) return;
            core -> when_parent_ready(node, target, [](godot::Node3D* n, godot::Node* t) {
                if (n -> get_parent() != t) n -> reparent(t, true);
            });
        }

        template<typename Instance, Type node_type = Type::Spatial>
        static void parent_methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "set_parent", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(entity = nil)", true)
                    .optional(2, [](Machine* vm, int idx) { return lua_isuserdata(vm -> get_state(), idx); });

                auto* node = self -> get_node();
                auto* core = Vital::Engine::Core::get_singleton();

                // Determine whether self is a server entity (ISyncable with net_id > 0).
                // NOTE: on the client this is also true for a REMOTE MIRROR of a server
                // entity — Network::_spawn_entity assigns the server's net_id to the
                // client-side copy too. A purely client-created model (API::Model::create()
                // called from client Lua) never gets a net_id, so this still correctly
                // tells the two apart on the client build.
                bool self_is_server = false;
                {
                    auto* syncable = dynamic_cast<Vital::Engine::ISyncable*>(node);
                    self_is_server  = (syncable && syncable->get_net_id() > 0);
                }

                #if defined(VSDK_Client)
                // Rule C: client Lua must NEVER reparent a server entity — attach or
                // detach. Engine::Model::set_parent() (the function that actually does
                // this safely: resets sync_parent_net_id, re-seeds the sync baseline,
                // and broadcasts _reparent_entity to every peer) doesn't even exist in
                // a client build, so this has to be rejected here, up front, before any
                // of the server-only logic below.
                if (self_is_server) {
                    throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
                        "set_parent: cannot be called on a server entity from the client");
                }
                #endif

                if (vm -> is_nil(2)) {
                    // Detach to Core root.
                    #if !defined(VSDK_Client)
                    if (self_is_server) {
                        // Must go through Engine::Model::set_parent() (same as the
                        // attach path below) so sync_parent_net_id resets to 0,
                        // sync_last_pos/rot re-seed in global space, and clients
                        // receive the _reparent_entity(net_id, 0) broadcast.
                        // A raw local reparent() here would silently desync every
                        // client, which would still think this entity is parented.
                        auto* model = dynamic_cast<Vital::Engine::Model*>(node);
                        if (model) model -> set_parent(nullptr);
                        else reparent_safe(core, node, core);
                    } else
                    #endif
                    {
                        // Rule B: client entity — purely local, no RPC needed.
                        reparent_safe(core, node, core);
                    }
                    vm -> push_value(true);
                    return 1;
                }

                auto* ud = vm_module::get_userdata_ptr(vm, 2);
                if (!ud || !*ud) { vm -> push_value(false); return 1; }

                auto* parent_node = static_cast<vm_instance_base*>(*ud) -> get_node_3d();
                if (!parent_node || parent_node == node || node -> is_ancestor_of(parent_node)) {
                    vm -> push_value(false);
                    return 1;
                }

                // Determine whether the requested parent is a server entity.
                // These are kept as two separate booleans (rather than folded into
                // one) purely so the error message below can tell apart two very
                // different situations that both fail the same "is it synced?"
                // test:
                //   - parent_is_syncable_type: the node's C++ type derives from
                //     ISyncable at all (Model, Physics_Body, ...). Types like
                //     Collision_Shape, Vehicle_Wheel, etc. never do — by design,
                //     they have no net_id and are intentionally excluded from the
                //     sync system (see collision_shape.h / vehicle_wheel.h). This
                //     is NOT a "client" node; it's simply not a syncable node type.
                //   - parent_is_server: it IS a syncable type, but its net_id is 0,
                //     meaning it's a genuine client-local instance (created by
                //     client Lua, never spawned/tracked on the server).
                auto* parent_syncable         = dynamic_cast<Vital::Engine::ISyncable*>(parent_node);
                bool  parent_is_syncable_type = (parent_syncable != nullptr);
                bool  parent_is_server        = (parent_syncable && parent_syncable->get_net_id() > 0);

                #if !defined(VSDK_Client)
                if (self_is_server) {
                    // Rule A: server entity → parent MUST also be a server entity.
                    // (Client code cannot reach this branch — rejected above.)
                    if (!parent_is_server) {
                        if (!parent_is_syncable_type) {
                            // e.g. Collision_Shape, Vehicle_Wheel: never synced by
                            // design, regardless of which side created them.
                            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
                                "set_parent: this node type is never network-synced and cannot be used "
                                "as a parent for a server entity (no net_id — e.g. Collision_Shape, "
                                "Vehicle_Wheel). Parent to the owning synced entity instead "
                                "(e.g. the Rigid_Body/Character/Static body, not its Collision_Shape).");
                        }
                        throw Tool::Log::fetch("request-failed", Tool::Log::Type::error,
                            "set_parent: server entity cannot be parented to a client-local entity "
                            "(net_id == 0 — this node was created by client Lua and never spawned on the server)");
                    }
                    // Delegate to Engine::Model::set_parent() which also broadcasts
                    // _reparent_entity to all clients.
                    auto* model = dynamic_cast<Vital::Engine::Model*>(node);
                    if (model) {
                        model->set_parent(parent_node);
                    } else {
                        // Fallback for non-Model server entities (physics bodies etc.)
                        // — plain local reparent; those types manage their own sync.
                        reparent_safe(core, node, parent_node);
                    }
                } else
                #endif
                {
                    // Rule B: client entity — parent may be anything (server or client,
                    // e.g. a camera or a client-created model attaching to a remote
                    // server entity). This is purely local; no RPC.
                    reparent_safe(core, node, parent_node);
                }

                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_parent", [](auto vm, auto self, auto& id) -> int {
                auto* parent = self -> get_node() -> get_parent();
                if (!parent || parent == Vital::Engine::Core::get_singleton()) {
                    vm -> push_value(false);
                    return 1;
                }
                std::lock_guard<std::mutex> lock(vm_node_registry_mutex);
                auto it = vm_node_registry.find(parent);
                if (it != vm_node_registry.end()) it -> second -> push_self(vm);
                else vm -> push_value(false);
                return 1;
            });
        }

        template<typename Instance, Type node_type = Type::Spatial>
        static void inject(Machine* vm) {}
    };
}
