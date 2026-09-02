/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: physics_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Physics Body Base (Syncable)
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/syncable.h>


///////////////////////////////
// Vital: Engine: PhysicsBody //
///////////////////////////////

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

    inline std::function<void(ISyncable*, PhysicsType, bool)> on_physics_body_spawned_callback;
    inline std::function<void(ISyncable*, PhysicsType)> on_physics_body_destroyed_callback;

    template<typename GodotBase>
    class PhysicsBodyBase : public GodotBase, public ISyncable {
        friend class Manager::Network;
        public:
            // ISyncable interface — shared implementation, same pattern as Model.
            SyncType get_sync_type() const override { return SyncType::PhysicsBody; }
            virtual PhysicsType get_physics_type() const = 0;

            // Returns the sub-type string used in the spawn RPC.
            // Derived from get_physics_type() — no dead body_sync_name field.
            virtual std::string get_sync_name() const override {
                switch (get_physics_type()) {
                    case PhysicsType::Rigid:       return "rigid";
                    case PhysicsType::Static:      return "static";
                    case PhysicsType::Character:   return "character";
                    case PhysicsType::Animatable:  return "animatable";
                    case PhysicsType::Vehicle:     return "vehicle";
                }
                return "";
            }

            bool is_sync_active() const override {
                return const_cast<PhysicsBodyBase*>(this) -> GodotBase::is_inside_tree() && net_id != 0;
            }

            godot::Vector3 get_sync_position() const override {
                return const_cast<PhysicsBodyBase*>(this) -> GodotBase::is_inside_tree() ? const_cast<PhysicsBodyBase*>(this) -> GodotBase::get_global_position() : godot::Vector3();
            }

            godot::Vector3 get_sync_rotation() const override {
                return const_cast<PhysicsBodyBase*>(this) -> GodotBase::is_inside_tree() ? const_cast<PhysicsBodyBase*>(this) -> GodotBase::get_rotation_degrees() : godot::Vector3();
            }

            void apply_sync(godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel) override {
                if (!GodotBase::is_inside_tree()) return;
                auto net = Manager::Network::get_singleton();
                if (net && net -> get_peer_id() == sync_authority) return;
                if (!interp_ready) {
                    GodotBase::set_global_position(pos);
                    GodotBase::set_rotation_degrees(rot);
                }
                sync_push_snapshot(pos, rot, vel);
                sync_last_pos = pos;
                sync_last_rot = rot;
                sync_last_vel = vel;
                sync_sleeping = false;
            }

            void on_sync_process(double delta) override {
                if (!GodotBase::is_inside_tree() || !interp_ready || net_id == 0) return;
                auto net = Manager::Network::get_singleton();
                if (net && net -> get_peer_id() == sync_authority) return;
                godot::Vector3 out_pos, out_rot;
                interp_process(delta, out_pos, out_rot);
                GodotBase::set_global_position(out_pos);
                GodotBase::set_rotation_degrees(out_rot);
            }

            void _notification(int what) {
                if (what == GodotBase::NOTIFICATION_PREDELETE) _notify_predelete_sync();
            }

            void destroy_sync() override { GodotBase::queue_free(); }

            void reset_sync_state() override {
                ISyncable::reset_sync_state();
                if constexpr (std::is_base_of_v<godot::RigidBody3D, GodotBase>) {
                    auto net = Manager::Network::get_singleton();
                    bool is_authority = net && net -> get_peer_id() == sync_authority;
                    GodotBase::set_freeze_mode(godot::RigidBody3D::FREEZE_MODE_KINEMATIC);
                    GodotBase::set_freeze_enabled(net_id == 0 ? false : !is_authority);
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
            // pending_authority is set before add_child so _ready() can call _ready_sync().
            int pending_authority = 1;

            void _ready_sync(int authority_peer) {
                sync_authority = authority_peer;
                sync_last_pos = GodotBase::get_global_position();
                sync_last_rot = GodotBase::get_rotation_degrees();
                sync_sleeping = false;
                sync_accum = 0.0f;
                reset_sync_state();
            }

            void _notify_predelete_sync() {
                if (on_physics_body_destroyed_callback) on_physics_body_destroyed_callback(this, get_physics_type());
                Manager::Network::get_singleton() -> unregister_syncable(this);
                sync_registered = false;
            }

            // Shared create/destroy logic — called by each derived body's static create()
            // and destroy() so those methods stay as thin wrappers around memnew/queue_free.
            //
            // setup_create(): called immediately after memnew(Derived) with authority_peer.
            //   Assigns net_id + pending_authority on the server path, enqueues the spawn
            //   RPC, and calls Core::add_child.  On the client path it just calls add_child.
            //   network.h is included by each derived .cpp so the Manager::Network calls
            //   resolve there rather than pulling a heavy header into this shared header.
            void setup_create(int authority_peer) {
                #if !defined(VSDK_Client)
                    if (authority_peer != 0) {
                        net_id            = next_net_id++;
                        pending_authority = authority_peer;
                        uint32_t captured_id   = net_id;
                        int      captured_auth = authority_peer;
                        std::string captured_name = get_sync_name();
                        Core::get_singleton() -> add_child(this);
                        Core::get_singleton() -> enqueue([this, captured_id, captured_auth, captured_name]() {
                            Manager::Network::get_singleton() -> enqueue_syncable_registration(this);
                            auto net_node = Manager::Network::get_singleton() -> get_node();
                            if (net_node) net_node -> rpc("_spawn_entity",
                                (int)captured_id,
                                (int)ISyncable::SyncType::PhysicsBody,
                                godot::String(captured_name.c_str()),
                                captured_auth);
                        });
                    }
                    else Core::get_singleton() -> add_child(this);
                #else
                    Core::get_singleton() -> add_child(this);
                #endif
            }

            // setup_destroy(): emits the destroy RPC on the server path, then queue_frees.
            void setup_destroy() {
                #if !defined(VSDK_Client)
                if (net_id != 0) {
                    auto net_node = Manager::Network::get_singleton() -> get_node();
                    if (net_node) net_node -> rpc("_destroy_entity", (int)net_id);
                }
                #endif
                GodotBase::queue_free();
            }

            PhysicsBodyBase() = default;
            ~PhysicsBodyBase() override = default;
    };
}
