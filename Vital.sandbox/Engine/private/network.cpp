/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: network.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/network.h>
#include <Vital.sandbox/Engine/public/model.h>
#include <Vital.sandbox/Engine/public/rigid_body.h>
#include <Vital.sandbox/Engine/public/static_body.h>
#include <Vital.sandbox/Engine/public/character_body.h>
#include <Vital.sandbox/Engine/public/animatable_body.h>
#include <Vital.sandbox/Engine/public/vehicle_body.h>
#include <Vital.sandbox/Engine/public/vehicle_wheel.h>
#include <Vital.sandbox/Engine/public/collision_shape.h>
#include <Vital.sandbox/API/physics/collision_shape.h>
#include <Vital.sandbox/Manager/public/network.h>
#include <Vital.sandbox/Manager/public/asset.h>


/////////////////////////////
// Vital: Engine: Network //
/////////////////////////////

// TODO: Improve?

namespace Vital::Engine {
    // Managers //
    void Network::setup_rpc() {
        godot::Dictionary reliable;
        reliable["rpc_mode"] = (int)godot::MultiplayerAPI::RPC_MODE_ANY_PEER;
        reliable["transfer_mode"] = (int)godot::MultiplayerPeer::TRANSFER_MODE_RELIABLE;
        reliable["call_local"] = false;
        reliable["channel"] = 0;
        rpc_config("_receive", reliable);
        rpc_config("_spawn_entity", reliable);
        rpc_config("_destroy_entity", reliable);
        rpc_config("_sync_state", reliable);
        rpc_config("_sync_config", reliable);
        rpc_config("_set_authority", reliable);
        rpc_config("_sync_shape", reliable);
        rpc_config("_reparent_entity", reliable);
        rpc_config("_spawn_wheel", reliable);
        rpc_config("_sync_wheel_config", reliable);
        rpc_config("_sync_wheel_transform", reliable);
        rpc_config("_sync_anim_layer", reliable);
        rpc_config("_sync_anim_layer_filter", reliable);
        rpc_config("_force_transform", reliable);
        rpc_config("_wake_sync", reliable);

        godot::Dictionary unreliable;
        unreliable["rpc_mode"] = (int)godot::MultiplayerAPI::RPC_MODE_ANY_PEER;
        unreliable["transfer_mode"] = (int)godot::MultiplayerPeer::TRANSFER_MODE_UNRELIABLE_ORDERED;
        unreliable["call_local"] = false;
        unreliable["channel"] = 0;
        rpc_config("_sync_entities", unreliable);
        rpc_config("_sync_client", unreliable);
    }


    // Signals //
    void Network::_receive(godot::Dictionary data) {
        Manager::Network::get_singleton() -> _on_packet_received(data);
    }
    
    void Network::_sync_entities(godot::PackedByteArray data) {
        Manager::Network::get_singleton() -> dispatch_sync_batch(data);
    }

    void Network::_sync_state(godot::PackedByteArray data) {
        Manager::Network::get_singleton() -> dispatch_sync_batch(data);
    }

    void Network::_sync_client(godot::PackedByteArray data) {
        #if !defined(VSDK_Client)
        auto tree = godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton() -> get_main_loop());
        int sender = tree ? tree -> get_multiplayer() -> get_remote_sender_id() : 0;
        Manager::Network::get_singleton() -> dispatch_client_sync(data, sender);
        #endif
    }

    void Network::_sync_config(int rate, float buffer_delay_max, float jitter_margin, float snap_threshold) {
        #if defined(VSDK_Client)
        Manager::Network::get_singleton() -> apply_sync_config(rate, buffer_delay_max, jitter_margin, snap_threshold);
        #endif
    }

    void Network::_wake_sync() {
        Manager::Network::get_singleton() -> wake_all_syncables();
    }

    void Network::_spawn_entity(int net_id, int type_id, godot::String name, int authority, godot::Vector3 init_pos, godot::Vector3 init_rot) {
        #if defined(VSDK_Client)
        using ST = Engine::ISyncable::SyncType;
        switch (static_cast<ST>(type_id)) {
            case ST::Model: {
                std::string model_name = Tool::to_std_string(name);
                auto it = Engine::Model::cache_loaded.find(model_name);
                Engine::Model* object = memnew(Engine::Model);
                object -> net_id = (uint32_t)net_id;
                object -> pending_authority = authority;
                object -> remote = true;

                if (it != Engine::Model::cache_loaded.end()) {
                    godot::Node* instance = it -> second -> instantiate();
                    if (instance) object -> add_child(instance);
                    object -> set_model_name(model_name);
                } 
                else {
                    object -> set_model_name(model_name);
                    object -> placeholder = true;
                    object -> set_visible(false);
                    Manager::Asset::get_singleton() -> queue_spawn(model_name, object, authority);
                }
                Engine::Core::get_singleton() -> add_child(object);
                // _ready() finds skeleton/anim_player, but only if not placeholder.
                // Call again explicitly so a same-frame _sync_anim_layer can see them.
                if (!object->placeholder) {
                    // find_node is private — _ready already ran via add_child.
                    // Re-run the same discovery _ready does by calling hydrate no-op path:
                    // actually _ready already did find_node. Nothing extra needed if
                    // instance was present before add_child (it was).
                }
                // Register immediately so get_entity_by_net_id() works inside
                // the entity:created Lua handler fired by on_spawned_callback.
                // register_syncable() is idempotent, so poll() will skip it.
                auto* net_mgr = Manager::Network::get_singleton();
                net_mgr -> register_syncable(object);
                // Apply server-supplied initial transform for remote bodies only.
                // Own-authority bodies position themselves — never overwrite.
                if (authority != net_mgr->get_peer_id()) {
                    object->set_global_position(init_pos);
                    object->set_rotation_degrees(init_rot);
                    object->delta_last_pos = init_pos;
                    object->delta_last_rot = init_rot;
                }
                net_mgr -> replay_pending_syncs(object);
                if (Engine::Model::on_spawned_callback) Engine::Model::on_spawned_callback(object, true);
                godot::UtilityFunctions::print("_spawn_entity [Model]: net_id=", net_id, " name=", name);
                break;
            }
            case ST::PhysicsBody: {
                std::string sub = Tool::to_std_string(name);
                Engine::ISyncable* entity = nullptr;
                Engine::PhysicsType sub_type = Engine::PhysicsType::Rigid;

                if (sub == "rigid_body") {
                    sub_type = Engine::PhysicsType::Rigid;
                    auto body = memnew(Engine::Rigid_Body);
                    Engine::Core::get_singleton() -> add_child(body);
                    entity = body;
                } 
                else if (sub == "static_body") {
                    sub_type = Engine::PhysicsType::Static;
                    auto body = memnew(Engine::Static_Body);
                    Engine::Core::get_singleton() -> add_child(body);
                    entity = body;
                } 
                else if (sub == "character_body") {
                    sub_type = Engine::PhysicsType::Character;
                    auto body = memnew(Engine::Character_Body);
                    Engine::Core::get_singleton() -> add_child(body);
                    entity = body;
                } 
                else if (sub == "animatable_body") {
                    sub_type = Engine::PhysicsType::Animatable;
                    auto body = memnew(Engine::Animatable_Body);
                    Engine::Core::get_singleton() -> add_child(body);
                    entity = body;
                } 
                else if (sub == "vehicle_body") {
                    sub_type = Engine::PhysicsType::Vehicle;
                    auto body = memnew(Engine::Vehicle_Body);
                    Engine::Core::get_singleton() -> add_child(body);
                    entity = body;
                } 
                else {
                    godot::UtilityFunctions::push_warning("_spawn_entity [PhysicsBody]: unknown sub-type=", name);
                    break;
                }

                if (entity) {
                    entity -> net_id = (uint32_t)net_id;
                    entity -> sync_authority = authority;
                    entity -> reset_sync_state();
                    // Register immediately so get_entity_by_net_id() works inside
                    // the entity:created Lua handler fired by on_spawned_callback.
                    // register_syncable() is idempotent, so poll() will skip it.
                    auto* net_mgr = Manager::Network::get_singleton();
                    net_mgr -> register_syncable(entity);
                    // Apply server-supplied initial transform for remote bodies only.
                    // Own-authority bodies position themselves via Lua — never overwrite.
                    if (authority != net_mgr->get_peer_id()) {
                        auto* node = entity->get_sync_node();
                        if (node) {
                            node->set_global_position(init_pos);
                            node->set_rotation_degrees(init_rot);
                        }
                        entity->delta_last_pos = init_pos;
                        entity->delta_last_rot = init_rot;
                    }
                    net_mgr -> replay_pending_syncs(entity);
                    godot::UtilityFunctions::print("_spawn_entity [PhysicsBody/", name, "]: net_id=", net_id);

                    // TODO: SHARE IN BETTER WAY?
                    // Notify Lua so it can hydrate collision shapes / wheels on
                    // this remote body — mirrors Model::on_spawned_callback.
                    if (Engine::on_spawned_callback) Engine::on_spawned_callback(entity, sub_type, true);
                }
                break;
            }
            default: {
                godot::UtilityFunctions::push_warning("_spawn_entity: unknown type_id=", type_id);
                break;
            }
        }
        #endif
    }

    void Network::_set_authority(int net_id, int peer_id) {
        #if defined(VSDK_Client)
        auto core = Engine::Core::get_singleton();
        if (!core) return;
        Engine::ISyncable* entity = Manager::Network::get_singleton() -> find_syncable((uint32_t)net_id);
        if (!entity) return;
        entity -> set_sync_authority(peer_id);
        godot::UtilityFunctions::print("_set_authority: net_id=", net_id, " -> peer_id=", peer_id);
        #endif
    }

    void Network::_destroy_entity(int net_id) {
        #if defined(VSDK_Client)
        Engine::ISyncable* entity = Manager::Network::get_singleton() -> find_syncable((uint32_t)net_id);
        if (!entity) return;
        entity -> destroy_sync();
        godot::UtilityFunctions::print("_destroy_entity: net_id=", net_id);
        #endif
    }

    void Network::_reparent_entity(int net_id, int parent_net_id) {
        #if defined(VSDK_Client)
        auto* mgr = Manager::Network::get_singleton();
        if (!mgr) return;

        // Try to apply immediately; if either side isn't registered yet, buffer.
        Engine::ISyncable* child_sync  = mgr->find_syncable((uint32_t)net_id);
        Engine::ISyncable* parent_sync = (parent_net_id != 0)
            ? mgr->find_syncable((uint32_t)parent_net_id) : nullptr;

        bool child_ready  = child_sync  != nullptr;
        bool parent_ready = (parent_net_id == 0) || (parent_sync != nullptr);

        if (child_ready && parent_ready) {
            apply_reparent_entity((uint32_t)net_id, (uint32_t)parent_net_id);
        } else {
            mgr->buffer_reparent((uint32_t)net_id, (uint32_t)parent_net_id);
            godot::UtilityFunctions::print(
                "_reparent_entity: buffering net_id=", net_id,
                " -> parent_net_id=", parent_net_id,
                " (child_ready=", child_ready, " parent_ready=", parent_ready, ")");
        }
        #endif
    }

    // Helper: given a net_id, return the VehicleBody3D node or nullptr.
    static godot::Node3D* find_vehicle_node(uint32_t net_id) {
        Engine::ISyncable* entity = Manager::Network::get_singleton()->find_syncable(net_id);
        if (!entity) return nullptr;
        return godot::Object::cast_to<godot::Node3D>(dynamic_cast<godot::Object*>(entity));
    }

    // Helper: find the wheel child by index.
    static Engine::Vehicle_Wheel* find_wheel(godot::Node3D* vehicle, int index) {
        for (int i = 0; i < vehicle->get_child_count(); i++) {
            auto* w = godot::Object::cast_to<Engine::Vehicle_Wheel>(vehicle->get_child(i));
            if (w && w->wheel_index == index) return w;
        }
        return nullptr;
    }

    // _spawn_wheel: client creates a VehicleWheel3D child on the matching vehicle body.
    // position/rotation are the wheel's local offset from the body center.
    void Network::_spawn_wheel(int net_id, int wheel_index, godot::Vector3 position, godot::Vector3 rotation) {
        #if defined(VSDK_Client)
        auto* vehicle = find_vehicle_node((uint32_t)net_id);
        if (!vehicle) return;

        // Don't double-create
        if (find_wheel(vehicle, wheel_index)) return;

        auto* wheel = memnew(Engine::Vehicle_Wheel);
        wheel->wheel_index = wheel_index;
        vehicle->add_child(wheel);
        wheel->set_position(position);
        wheel->set_rotation(rotation);
        godot::UtilityFunctions::print("_spawn_wheel: net_id=", net_id, " wheel_index=", wheel_index);
        #endif
    }

    // _sync_wheel_config: client applies a single setup-time property to a wheel.
    // Per-tick inputs (engine_force, brake, steering) are NOT sent — the authority
    // peer runs those locally; the result is captured in the body transform sync.
    void Network::_sync_wheel_config(int net_id, int wheel_index, godot::String key, godot::Variant value) {
        #if defined(VSDK_Client)
        auto* vehicle = find_vehicle_node((uint32_t)net_id);
        if (!vehicle) return;
        auto* wheel = find_wheel(vehicle, wheel_index);
        if (!wheel) return;

        std::string k = Tool::to_std_string(key);
        if      (k == "radius")                   wheel->set_radius((float)value);
        else if (k == "suspension_rest_length")   wheel->set_suspension_rest_length((float)value);
        else if (k == "suspension_travel")        wheel->set_suspension_travel((float)value);
        else if (k == "suspension_stiffness")     wheel->set_suspension_stiffness((float)value);
        else if (k == "suspension_max_force")     wheel->set_suspension_max_force((float)value);
        else if (k == "damping_compression")      wheel->set_damping_compression((float)value);
        else if (k == "damping_relaxation")       wheel->set_damping_relaxation((float)value);
        else if (k == "use_as_traction")          wheel->set_use_as_traction((bool)value);
        else if (k == "use_as_steering")          wheel->set_use_as_steering((bool)value);
        else if (k == "friction_slip")            wheel->set_friction_slip((float)value);
        else if (k == "roll_influence")           wheel->set_roll_influence((float)value);
        else godot::UtilityFunctions::push_warning("_sync_wheel_config: unknown key: ", key);
        #endif
    }


    // _sync_wheel_transform: client applies local position + rotation to a wheel.
    // Used for customization (repositioning, camber, tire size changes etc.).
    // Always local-space — wheels are children of the vehicle body.
    void Network::_sync_wheel_transform(int net_id, int wheel_index, godot::Vector3 position, godot::Vector3 rotation) {
        #if defined(VSDK_Client)
        auto* vehicle = find_vehicle_node((uint32_t)net_id);
        if (!vehicle) return;
        auto* wheel = find_wheel(vehicle, wheel_index);
        if (!wheel) return;
        wheel->set_position(position);
        wheel->set_rotation(rotation);
        #endif
    }


    // _sync_anim_layer: replicates Model animation-layer state (see
    // Model::broadcast_animation_layer). mode: 0=play, 1=stop, 2=set weight,
    // 3=set speed.
    //
    // Server: relays whichever peer currently holds sync authority over the
    // target model — anyone else's packet for that net_id is dropped as a
    // spoof attempt — then rebroadcasts reliably to every connected client
    // (same relay shape as _sync_client uses for client-authoritative
    // movement). Server build never applies the animation locally; Model
    // only carries a live AnimationPlayer/AnimationTree on the client.
    //
    // Client: applies the animation state to the matching Model directly via
    // its private apply_*() helpers (Network is a friend of Model) — never
    // through the public play_animation_layer()/etc, which would re-trigger
    // another broadcast and echo the packet back onto the network.
    void Network::_sync_anim_layer(int net_id, int layer, int mode, godot::String name, bool loop, float speed, float weight, float blend_time) {
        auto* mgr = Manager::Network::get_singleton();
        if (!mgr) return;
        Engine::ISyncable* entity = mgr->find_syncable((uint32_t)net_id);
        if (!entity) return;

        #if !defined(VSDK_Client)
        auto tree = godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton()->get_main_loop());
        int sender = tree ? tree->get_multiplayer()->get_remote_sender_id() : 0;
        if (sender != 0 && sender != entity->get_sync_authority()) {
            godot::UtilityFunctions::push_warning("_sync_anim_layer: rejected — sender ", sender,
                " is not the sync authority for net_id=", net_id);
            return;
        }
        auto* node = mgr->get_node();
        if (node) node->rpc("_sync_anim_layer", net_id, layer, mode, name, loop, speed, weight, blend_time);
        #else
        auto* model = godot::Object::cast_to<Engine::Model>(dynamic_cast<godot::Object*>(entity));
        if (!model) return;

        std::string std_name = Tool::to_std_string(name);
        switch (mode) {
            case 0: model->apply_play_animation_layer(layer, std_name, loop, speed, weight, blend_time); break;
            case 1: model->apply_stop_animation_layer(layer, blend_time); break;
            case 2: model->apply_set_animation_layer_weight(layer, weight, blend_time); break;
            case 3: model->apply_set_animation_layer_speed(layer, speed); break;
            default: godot::UtilityFunctions::push_warning("_sync_anim_layer: unknown mode=", mode); break;
        }
        #endif
    }


    // _force_transform: server → owning peer (reliable, rpc_id).
    // Applies a server-mandated position/rotation to an entity regardless of
    // whether the receiving peer holds sync authority over it.  Unlike the
    // unreliable _sync_entities batch, apply_sync() will NOT drop this on the
    // authority peer — we write directly to the node and reseed the sync
    // baseline so the client's next _sync_client upload starts from the new
    // origin instead of snapping the entity back to wherever it was before.
    // The server also calls broadcast_sync() separately for all OTHER clients,
    // so this RPC only needs to handle the owning peer's side.
    void Network::_sync_anim_layer_filter(int net_id, int layer, bool enabled, godot::PackedStringArray bones) {
        auto* mgr = Manager::Network::get_singleton();
        if (!mgr) return;
        Engine::ISyncable* entity = mgr->find_syncable((uint32_t)net_id);
        if (!entity) return;

        #if !defined(VSDK_Client)
        auto tree = godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton()->get_main_loop());
        int sender = tree ? tree->get_multiplayer()->get_remote_sender_id() : 0;
        if (sender != 0 && sender != entity->get_sync_authority()) {
            godot::UtilityFunctions::push_warning("_sync_anim_layer_filter: rejected — sender ", sender,
                " is not the sync authority for net_id=", net_id);
            return;
        }
        // Persist on server model state if present (for late-join dump).
        if (auto* model = godot::Object::cast_to<Engine::Model>(dynamic_cast<godot::Object*>(entity))) {
            std::vector<std::string> paths;
            paths.reserve(bones.size());
            for (int i = 0; i < bones.size(); ++i)
                paths.push_back(Tool::to_std_string(bones[i]));
            model->apply_set_animation_layer_filter(layer, enabled, paths);
        }
        auto* node = mgr->get_node();
        if (node) node->rpc("_sync_anim_layer_filter", net_id, layer, enabled, bones);
        #else
        auto* model = godot::Object::cast_to<Engine::Model>(dynamic_cast<godot::Object*>(entity));
        if (!model) return;
        std::vector<std::string> paths;
        paths.reserve(bones.size());
        for (int i = 0; i < bones.size(); ++i)
            paths.push_back(Tool::to_std_string(bones[i]));
        model->apply_set_animation_layer_filter(layer, enabled, paths);
        #endif
    }


    void Network::_force_transform(int net_id, godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 scale) {
        #if defined(VSDK_Client)
        auto* mgr = Manager::Network::get_singleton();
        if (!mgr) return;
        Engine::ISyncable* entity = mgr->find_syncable((uint32_t)net_id);
        if (!entity) {
            // Entity not registered yet — buffer and replay from replay_pending_syncs().
            mgr->buffer_force_transform((uint32_t)net_id, pos, rot, scale);
            return;
        }

        // Force arrived before the matching _reparent_entity was applied (common
        // when set_parent + set_position run in the same server Lua tick: force
        // RPC is sent immediately, reparent RPC is enqueued a frame later).
        // Buffer and let replay / post-reparent force apply it in the correct
        // parent-relative space after reparent zeros local position.
        if (entity->get_sync_parent_net_id() == 0 && mgr->has_pending_reparent((uint32_t)net_id)) {
            mgr->buffer_force_transform((uint32_t)net_id, pos, rot, scale);
            godot::UtilityFunctions::print(
                "_force_transform: buffering until reparent net_id=", net_id, " pos=", pos);
            return;
        }

        auto* node = entity->get_sync_node();
        if (!node) return;

        // Parent-relative or global depending on sync coordinate space.
        if (entity->get_sync_parent_net_id() != 0) {
            node->set_position(pos);
            node->set_rotation_degrees(rot);
        } else {
            node->set_global_position(pos);
            node->set_rotation_degrees(rot);
        }
        node->set_scale(scale);

        // Reseed baselines so this client's next _sync_client packet starts
        // from the new position.  Without this the client's upload would carry
        // a delta relative to the OLD origin, and the server would relay that
        // bad delta to every other peer — undoing the correction we just made.
        entity->sync_last_pos  = pos;
        entity->sync_last_rot  = rot;
        entity->sync_last_vel  = godot::Vector3();
        entity->delta_last_pos = pos;
        entity->delta_last_rot = rot;
        entity->delta_last_vel = godot::Vector3();
        entity->sync_sleeping  = false;
        entity->sync_accum     = 0.0f;
        godot::UtilityFunctions::print("_force_transform: net_id=", net_id, " pos=", pos);
        #endif
    }

    // _sync_shape: called on clients when server assigns/changes a collision shape on a synced body.
    // Finds or creates our Engine::Collision_Shape child on the matching body node and applies the shape.
    // Vehicle wheels are children of their vehicle body and driven by Godot's physics — no sync needed.
    void Network::_sync_shape(int net_id, godot::String shape_type, godot::Array params) {
        #if defined(VSDK_Client)
        // The server sends this the moment a shape is set — which can be the same
        // script tick the body was created, before the body's own spawn RPC has
        // even reached the network (Rigid_Body::create defers its registration+RPC
        // by a frame). If the body isn't registered here yet, buffer this shape and
        // let Manager::Network::poll() replay it once the net_id shows up.
        Engine::ISyncable* entity = Manager::Network::get_singleton() -> find_syncable((uint32_t)net_id);
        if (!entity) {
            Manager::Network::get_singleton() -> defer_shape_sync((uint32_t)net_id, shape_type, params);
            return;
        }
        apply_shape((uint32_t)net_id, shape_type, params);
        #endif
    }

    void Network::apply_shape(uint32_t net_id, godot::String shape_type, godot::Array params) {
        #if defined(VSDK_Client)
        Engine::ISyncable* entity = Manager::Network::get_singleton() -> find_syncable(net_id);
        if (!entity) return;

        auto node = godot::Object::cast_to<godot::Node3D>(dynamic_cast<godot::Object*>(entity));
        if (!node) return;

        // Find existing Collision_Shape child or create one
        Engine::Collision_Shape* col = nullptr;
        for (int i = 0; i < node -> get_child_count(); i++) {
            col = godot::Object::cast_to<Engine::Collision_Shape>(node -> get_child(i));
            if (col) break;
        }
        if (!col) {
            col = memnew(Engine::Collision_Shape);
            node -> add_child(col);
            if (Engine::Collision_Shape::on_spawned_callback) Engine::Collision_Shape::on_spawned_callback(col);
        }

        // FIXED: now routes through Collision_Shape::assign_shape(), which sets
        // the native shape AND (client-only) keeps current_shape + the debug
        // wireframe in sync itself. Previously this called set_shape() directly
        // and then had to separately walk the Lua API registry to find a
        // matching Instance and manually call refresh_debug_mesh() on it — that
        // walk relied on the Instance already existing at this exact point,
        // which wasn't guaranteed, and it's also just redundant work now that
        // the engine node owns its own debug state.
        std::string type = Tool::to_std_string(shape_type);
        if (type == "box" && params.size() >= 3) {
            godot::Ref<godot::BoxShape3D> s; s.instantiate();
            s -> set_size(godot::Vector3((float)params[0], (float)params[1], (float)params[2]));
            col -> assign_shape(s);
        }
        else if (type == "sphere" && params.size() >= 1) {
            godot::Ref<godot::SphereShape3D> s; s.instantiate();
            s -> set_radius((float)params[0]);
            col -> assign_shape(s);
        }
        else if (type == "capsule" && params.size() >= 2) {
            godot::Ref<godot::CapsuleShape3D> s; s.instantiate();
            s -> set_radius((float)params[0]);
            s -> set_height((float)params[1]);
            col -> assign_shape(s);
        }
        else if (type == "cylinder" && params.size() >= 2) {
            godot::Ref<godot::CylinderShape3D> s; s.instantiate();
            s -> set_radius((float)params[0]);
            s -> set_height((float)params[1]);
            col -> assign_shape(s);
        }
        else if (type == "world_boundary" && params.size() >= 4) {
            godot::Ref<godot::WorldBoundaryShape3D> s; s.instantiate();
            s -> set_plane(godot::Plane(godot::Vector3((float)params[0], (float)params[1], (float)params[2]), (float)params[3]));
            col -> assign_shape(s);
        }
        else if (type == "separation_ray" && params.size() >= 1) {
            godot::Ref<godot::SeparationRayShape3D> s; s.instantiate();
            s -> set_length((float)params[0]);
            col -> assign_shape(s);
        }
        else {
            godot::UtilityFunctions::push_warning("_sync_shape: unknown type or bad params: ", shape_type);
        }
        #endif
    }

    // _reparent_entity — server → all clients
    // Moves the remote node for net_id under the remote node for parent_net_id
    // (or back to Core root when parent_net_id == 0).
    // Only the client-side scene tree is touched; the server already performed
    // the real reparent before broadcasting this RPC.
    //
    // Both the child and parent may not be in sync_id_map yet when this RPC
    // arrives (e.g. if apply_reparent_entity races with _spawn_entity on the
    // same frame).  If either is missing we buffer the reparent in
    // pending_reparent_syncs; poll() replays it once both register.
    void Network::apply_reparent_entity(uint32_t net_id, uint32_t parent_net_id) {
        auto* mgr  = Manager::Network::get_singleton();
        auto* core = Engine::Core::get_singleton();
        if (!mgr || !core) return;

        Engine::ISyncable* child_sync = mgr->find_syncable(net_id);
        if (!child_sync) return;
        auto* child_node = godot::Object::cast_to<godot::Node3D>(
            dynamic_cast<godot::Object*>(child_sync));
        if (!child_node) return;

        godot::Node* target = core;
        if (parent_net_id != 0) {
            Engine::ISyncable* parent_sync = mgr->find_syncable(parent_net_id);
            if (!parent_sync) return;  // caller must retry
            auto* parent_node = godot::Object::cast_to<godot::Node3D>(
                dynamic_cast<godot::Object*>(parent_sync));
            if (!parent_node) return;
            target = parent_node;
        }

        if (child_node->is_inside_tree() && child_node->get_parent() != target) {
            // Match server: keep world transform so prior force/spawn placement
            // is not wiped. Local offset is corrected by state dump / force.
            child_node->reparent(target, true);
        }

        // Switch sync coordinate space for every ISyncable type (Model,
        // Physics_Body subtypes, or anything added later).
        // set_sync_parent_net_id is defined once on ISyncable — no per-type
        // casting required here.  Baseline re-seed must happen AFTER reparent
        // so get_sync_position/rotation already return local-space values.
        child_sync->set_sync_parent_net_id(parent_net_id);
        child_sync->sync_last_pos  = child_sync->get_sync_position();
        child_sync->sync_last_rot  = child_sync->get_sync_rotation();
        child_sync->sync_last_vel  = godot::Vector3();
        child_sync->delta_last_pos = child_sync->sync_last_pos;
        child_sync->delta_last_rot = child_sync->sync_last_rot;
        child_sync->delta_last_vel = godot::Vector3();

#if defined(VSDK_Client)
        // If a _force_transform was buffered because it arrived before this
        // reparent (or while reparent was still pending), apply it now that
        // we are in the correct parent-relative coordinate space.  Without
        // this, the zeroed local position above would stick until a later
        // force arrives — which is exactly the "local controller needs a
        // timer" bug.  Helpers only exist on the client build.
        {
            godot::Vector3 fpos, frot;
            godot::Vector3 fscale = godot::Vector3(1,1,1);
            if (mgr->take_pending_force_transform(net_id, fpos, frot, fscale)) {
                child_node->set_position(fpos);
                child_node->set_rotation_degrees(frot);
                child_node->set_scale(fscale);
                child_sync->sync_last_pos  = fpos;
                child_sync->sync_last_rot  = frot;
                child_sync->delta_last_pos = fpos;
                child_sync->delta_last_rot = frot;
                child_sync->sync_sleeping  = false;
                child_sync->sync_accum     = 0.0f;
                godot::UtilityFunctions::print(
                    "_reparent_entity: applied buffered force net_id=", net_id, " pos=", fpos);
            }
        }
#endif

        godot::UtilityFunctions::print(
            "_reparent_entity: net_id=", net_id,
            " -> parent_net_id=", parent_net_id);
    }
    
    #if defined(VSDK_Client)
    void Network::_on_connected_to_server() { if (on_connected_to_server) on_connected_to_server(); }
    void Network::_on_connection_failed() { if (on_connection_failed) on_connection_failed(); }
    void Network::_on_server_disconnected() { if (on_server_disconnected) on_server_disconnected(); }
    #else
    void Network::_on_peer_connected(int id) { if (on_peer_connected) on_peer_connected(id); }
    void Network::_on_peer_disconnected(int id) { if (on_peer_disconnected) on_peer_disconnected(id); }
    #endif
}
