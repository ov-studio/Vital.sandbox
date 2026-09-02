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
        rpc_config("_spawn_wheel", reliable);
        rpc_config("_sync_wheel_config", reliable);
        rpc_config("_sync_wheel_transform", reliable);
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
        Manager::Network::get_singleton() -> dispatch_sync_batch(data, false);
    }

    void Network::_sync_state(godot::PackedByteArray data) {
        Manager::Network::get_singleton() -> dispatch_sync_batch(data, true);
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

    void Network::_spawn_entity(int net_id, int type_id, godot::String name, int authority) {
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
                    Manager::Asset::get_singleton() -> queue_spawn(model_name, object);
                }
                Engine::Core::get_singleton() -> add_child(object);
                Manager::Network::get_singleton() -> enqueue_syncable_registration(object);
                if (Engine::Model::on_spawned_callback) Engine::Model::on_spawned_callback(object, true);
                godot::UtilityFunctions::print("_spawn_entity [Model]: net_id=", net_id, " name=", name);
                break;
            }
            case ST::PhysicsBody: {
                std::string sub = Tool::to_std_string(name);
                Engine::ISyncable* entity = nullptr;
                Engine::PhysicsType sub_type = Engine::PhysicsType::Rigid;  // TODO: empty no init maybe?

                if (sub == "rigid") {
                    sub_type = Engine::PhysicsType::Rigid;
                    auto body = memnew(Engine::Rigid_Body);
                    Engine::Core::get_singleton() -> add_child(body);
                    entity = body;
                } 
                else if (sub == "static") {
                    sub_type = Engine::PhysicsType::Static;
                    auto body = memnew(Engine::Static_Body);
                    Engine::Core::get_singleton() -> add_child(body);
                    entity = body;
                } 
                else if (sub == "character") {
                    sub_type = Engine::PhysicsType::Character;
                    auto body = memnew(Engine::Character_Body);
                    Engine::Core::get_singleton() -> add_child(body);
                    entity = body;
                } 
                else if (sub == "animatable") {
                    sub_type = Engine::PhysicsType::Animatable;
                    auto body = memnew(Engine::Animatable_Body);
                    Engine::Core::get_singleton() -> add_child(body);
                    entity = body;
                } 
                else if (sub == "vehicle") {
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
                    Manager::Network::get_singleton() -> enqueue_syncable_registration(entity);
                    godot::UtilityFunctions::print("_spawn_entity [PhysicsBody/", name, "]: net_id=", net_id);

                    // TODO: SHARE IN BETTER WAY?
                    // Notify Lua so it can hydrate collision shapes / wheels on
                    // this remote body — mirrors Model::on_spawned_callback.
                    if (Engine::on_physics_body_spawned_callback)
                        Engine::on_physics_body_spawned_callback(entity, sub_type, true);
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
            if (Engine::Collision_Shape::on_spawned_callback)
                Engine::Collision_Shape::on_spawned_callback(col);
        }

        std::string type = Tool::to_std_string(shape_type);
        godot::Ref<godot::Shape3D> applied_shape;
        if (type == "box" && params.size() >= 3) {
            godot::Ref<godot::BoxShape3D> s; s.instantiate();
            s -> set_size(godot::Vector3((float)params[0], (float)params[1], (float)params[2]));
            col -> set_shape(s);
            applied_shape = s;
        }
        else if (type == "sphere" && params.size() >= 1) {
            godot::Ref<godot::SphereShape3D> s; s.instantiate();
            s -> set_radius((float)params[0]);
            col -> set_shape(s);
            applied_shape = s;
        }
        else if (type == "capsule" && params.size() >= 2) {
            godot::Ref<godot::CapsuleShape3D> s; s.instantiate();
            s -> set_radius((float)params[0]);
            s -> set_height((float)params[1]);
            col -> set_shape(s);
            applied_shape = s;
        }
        else if (type == "cylinder" && params.size() >= 2) {
            godot::Ref<godot::CylinderShape3D> s; s.instantiate();
            s -> set_radius((float)params[0]);
            s -> set_height((float)params[1]);
            col -> set_shape(s);
            applied_shape = s;
        }
        else if (type == "world_boundary" && params.size() >= 4) {
            godot::Ref<godot::WorldBoundaryShape3D> s; s.instantiate();
            s -> set_plane(godot::Plane(godot::Vector3((float)params[0], (float)params[1], (float)params[2]), (float)params[3]));
            col -> set_shape(s);
            applied_shape = s;
        }
        else if (type == "separation_ray" && params.size() >= 1) {
            godot::Ref<godot::SeparationRayShape3D> s; s.instantiate();
            s -> set_length((float)params[0]);
            col -> set_shape(s);
            applied_shape = s;
        }
        else {
            godot::UtilityFunctions::push_warning("_sync_shape: unknown type or bad params: ", shape_type);
        }

        // Sync current_shape on the API Instance so refresh_debug_mesh() can build
        // the wireframe. Without this the debug mesh stays blank because the Instance
        // never sees the shape that was set on the engine node above.
        if (applied_shape.is_valid()) {
            std::lock_guard<std::mutex> lock(Sandbox::API::Collision_Shape::registry.mutex);
            for (auto& [id, inst] : Sandbox::API::Collision_Shape::registry.buffer) {
                if (inst->body == col) {
                    inst->current_shape = applied_shape;
                    inst->refresh_debug_mesh();
                    break;
                }
            }
        }
        #endif
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
