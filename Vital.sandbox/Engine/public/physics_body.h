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

// TODO: Improve and all physics body abstract more if possible
// Sub-type tag packed into the _spawn_entity RPC so clients know which
// Godot class to instantiate.  Fits in one byte (uint8_t).
// NOTE: SyncType::PhysicsBody (=1) is the ISyncable type; PhysicsSubType
// below disambiguates among the four physics body classes.
namespace Vital::Manager { class Network; }

namespace Vital::Engine {
    // Sub-type used in spawn RPC alongside SyncType::PhysicsBody.
    enum class PhysicsSubType : uint8_t {
        Rigid       = 0,
        Static      = 1,
        Character   = 2,
        Animatable  = 3,
        Vehicle     = 4,
    };

    // Base mixin — owns sync state and fulfils ISyncable for any physics body.
    // Concrete classes inherit this *and* the appropriate Godot body class.
    // Server-created bodies are network-replicated (ISyncable). Client-created
    // bodies (net_id == 0) are local-only — never synced.
    template<typename GodotBase>
    class PhysicsBodyBase : public GodotBase, public ISyncable {
        friend class Manager::Network;
    public:
        // ISyncable interface — shared implementation, same pattern as Model.
        SyncType get_sync_type() const override { return SyncType::PhysicsBody; }
        virtual PhysicsSubType get_physics_sub_type() const = 0;
        virtual std::string get_sync_name() const override { return body_sync_name; }

        bool is_sync_active() const override {
            return const_cast<PhysicsBodyBase*>(this)->GodotBase::is_inside_tree() && net_id != 0;
        }

        godot::Vector3 get_sync_position() const override {
            return const_cast<PhysicsBodyBase*>(this)->GodotBase::is_inside_tree()
                ? const_cast<PhysicsBodyBase*>(this)->GodotBase::get_global_position()
                : godot::Vector3();
        }

        godot::Vector3 get_sync_rotation() const override {
            return const_cast<PhysicsBodyBase*>(this)->GodotBase::is_inside_tree()
                ? const_cast<PhysicsBodyBase*>(this)->GodotBase::get_rotation_degrees()
                : godot::Vector3();
        }

        void apply_sync(godot::Vector3 pos, godot::Vector3 rot, godot::Vector3 vel) override {
            if (!GodotBase::is_inside_tree()) return;
            auto net = Manager::Network::get_singleton();
            if (net && net->get_peer_id() == sync_authority) return; // authority drives raw
            sync_push_snapshot(pos, rot, vel);
            sync_last_pos = pos;
            sync_last_rot = rot;
            sync_last_vel = vel;
            sync_sleeping = false;
        }

        void on_sync_process(double delta) override {
            if (!GodotBase::is_inside_tree() || !interp_ready || net_id == 0) return;
            auto net = Manager::Network::get_singleton();
            if (net && net->get_peer_id() == sync_authority) return;
            godot::Vector3 out_pos, out_rot;
            interp_process(delta, out_pos, out_rot);
            GodotBase::set_global_position(out_pos);
            GodotBase::set_rotation_degrees(out_rot);
        }

        void destroy_sync() override { GodotBase::queue_free(); }

        uint32_t get_net_id() const override { return net_id; }
        int get_sync_authority() const override { return sync_authority; }

        // Server-only: assign authority peer for this body.
        // net_id == 0 means client-local, never synced — set_syncer is no-op.
        #if !defined(VSDK_Client)
        void set_syncer(int peer_id) {
            if (net_id == 0) return; // local body — not replicated
            sync_authority = (peer_id <= 1) ? 1 : peer_id;
            sync_sleeping  = false;
            auto net_node = Manager::Network::get_singleton()->get_node();
            if (net_node) net_node->rpc("_set_authority", (int)net_id, sync_authority);
        }
        #endif

    protected:
        std::string body_sync_name; // set on creation for sub-type identification

        void _ready_sync(int authority_peer) {
            sync_authority = authority_peer;
            sync_last_pos  = GodotBase::get_global_position();
            sync_last_rot  = GodotBase::get_rotation_degrees();
            sync_sleeping  = false;
            sync_accum     = 0.0f;
            reset_sync_state();
        }

        void _notify_predelete_sync() {
            Manager::Network::get_singleton()->unregister_syncable(this);
            sync_registered = false;
        }

        PhysicsBodyBase() = default;
        ~PhysicsBodyBase() override = default;
    };
}
