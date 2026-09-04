/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: physics_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Physics Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/syncable.h>


//////////////////////////////////
// Vital: Engine: Physics_Body //
//////////////////////////////////

// TODO: Improve 
namespace Vital::Manager { 
    class Network;
}

namespace Vital::Engine {
    enum class PhysicsType {
        Rigid,
        Static,
        Character,
        Animatable,
        Vehicle,
    };

    inline std::function<void(ISyncable*, PhysicsType, bool)> on_spawned_callback;
    inline std::function<void(ISyncable*, PhysicsType)> on_destroyed_callback;

    template<typename Base>
    class Physics_Body : public Base, public ISyncable {
        friend class Manager::Network;
        public:
            SyncType get_sync_type() const override { 
                return SyncType::PhysicsBody; 
            }

            virtual PhysicsType get_physics_type() const = 0;

            virtual std::string get_sync_name() const override {
                switch (get_physics_type()) {
                    case PhysicsType::Rigid:      return "rigid_body";
                    case PhysicsType::Static:     return "static_body";
                    case PhysicsType::Character:  return "character_body";
                    case PhysicsType::Animatable: return "animatable_body";
                    case PhysicsType::Vehicle:    return "vehicle_body";
                }
                return "";
            }

            bool is_sync_active() const override {
                return const_cast<Physics_Body*>(this) -> Base::is_inside_tree() && net_id != 0;
            }

            godot::Vector3 get_sync_position() const override {
                return const_cast<Physics_Body*>(this) -> Base::is_inside_tree() ? const_cast<Physics_Body*>(this) -> Base::get_global_position() : godot::Vector3();
            }

            godot::Vector3 get_sync_rotation() const override {
                return const_cast<Physics_Body*>(this) -> Base::is_inside_tree() ? const_cast<Physics_Body*>(this) -> Base::get_rotation_degrees() : godot::Vector3();
            }

            void apply_sync(godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel) override {
                if (!Base::is_inside_tree()) return;
                auto net = Manager::Network::get_singleton();
                if (net && net -> get_peer_id() == sync_authority) return;
                if (!interp_ready) {
                    Base::set_global_position(pos);
                    Base::set_rotation_degrees(rot);
                }
                sync_push_snapshot(pos, rot, vel);
                sync_last_pos = pos;
                sync_last_rot = rot;
                sync_last_vel = vel;
                sync_sleeping = false;
            }

            void on_sync_process(double delta) override {
                if (!Base::is_inside_tree() || !interp_ready || net_id == 0) return;
                auto net = Manager::Network::get_singleton();
                if (net && net -> get_peer_id() == sync_authority) return;
                godot::Vector3 out_pos, out_rot;
                interp_process(delta, out_pos, out_rot);
                Base::set_global_position(out_pos);
                Base::set_rotation_degrees(out_rot);
            }

            void _notification(int what) {
                if (what == Base::NOTIFICATION_PREDELETE) _notify_predelete_sync();
            }

            void destroy_sync() override { Base::queue_free(); }
            godot::Node3D* get_sync_node() override { return this; }

            void reset_sync_state() override {
                ISyncable::reset_sync_state();
                if constexpr (std::is_base_of_v<godot::RigidBody3D, Base>) {
                    auto net = Manager::Network::get_singleton();
                    bool is_authority = net && net -> get_peer_id() == sync_authority;
                    Base::set_freeze_mode(godot::RigidBody3D::FREEZE_MODE_KINEMATIC);
                    Base::set_freeze_enabled(net_id == 0 ? false : !is_authority);
                }
            }

            uint32_t get_net_id() const override { return net_id; }
            int get_sync_authority() const override { return sync_authority; }

            // Server-only: assign authority peer for this body.
            // net_id == 0 means client-local, never synced — set_syncer is no-op.
            #if !defined(VSDK_Client)
            void set_syncer(int peer_id) {
                if (net_id == 0) return;
                sync_authority = (peer_id <= 1) ? 1 : peer_id;
                sync_sleeping  = false;
                reset_sync_state();
                auto net_node = Manager::Network::get_singleton() -> get_node();
                if (net_node) net_node -> rpc("_set_authority", (int)net_id, sync_authority);
            }
            #endif
        protected:
            int pending_authority = 1;


            // Instantiators //
            Physics_Body() = default;
            ~Physics_Body() override = default;

            
            // Hooks //
            void _ready_sync(int authority_peer) {
                sync_authority = authority_peer;
                sync_last_pos = Base::get_global_position();
                sync_last_rot = Base::get_rotation_degrees();
                sync_sleeping = false;
                sync_accum = 0.0f;
                reset_sync_state();
            }

            void _notify_predelete_sync() {
                if (on_destroyed_callback) on_destroyed_callback(this, get_physics_type());
                Manager::Network::get_singleton() -> unregister_syncable(this);
                sync_registered = false;
            }


            // Managers //
            void setup_create(int authority_peer) {
                #if !defined(VSDK_Client)
                    if (authority_peer != 0) {
                        net_id = next_net_id++;
                        pending_authority = authority_peer;
                        uint32_t captured_id = net_id;
                        int captured_auth = authority_peer;
                        godot::String captured_name = godot::String(get_sync_name().c_str());
                        Core::get_singleton() -> add_child(this);
                        Core::get_singleton() -> enqueue([this, captured_id, captured_auth, captured_name]() {
                            Manager::Network::get_singleton() -> enqueue_syncable_registration(this);
                            auto net_node = Manager::Network::get_singleton() -> get_node();
                            if (net_node) net_node -> rpc("_spawn_entity", (int)captured_id, (int)ISyncable::SyncType::PhysicsBody, captured_name, captured_auth);
                        });
                    }
                    else Core::get_singleton() -> add_child(this);
                #else
                    Core::get_singleton() -> add_child(this);
                #endif
            }

            void setup_destroy() {
                #if !defined(VSDK_Client)
                if (net_id != 0) {
                    auto net_node = Manager::Network::get_singleton() -> get_node();
                    if (net_node) net_node -> rpc("_destroy_entity", (int)net_id);
                }
                #endif
                Base::queue_free();
            }
    };
}
