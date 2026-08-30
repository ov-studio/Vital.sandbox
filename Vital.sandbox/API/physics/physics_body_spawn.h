/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: physics: physics_body_spawn.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Physics body remote-spawn callback wiring.
           Included by apis.h after all body API headers so every
           body type's Instance and registry are fully defined.
           Registers one global callback that creates the correct
           Lua-facing Instance for any remote body spawned by
           _spawn_entity, mirroring Model::on_spawned_callback.

           Also registers the destroy-side counterpart: fired from
           PhysicsBodyBase::_notify_predelete_sync() (NOTIFICATION_PREDELETE)
           for ANY physics body teardown — local ->destroy(), remote
           _destroy_entity RPC via destroy_sync(), or otherwise — so the
           Lua-facing Instance is always dropped and entity:destroyed
           always fires, mirroring Model::on_destroyed_callback.
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/physics_body.h>
#include <Vital.sandbox/API/physics/rigid_body.h>
#include <Vital.sandbox/API/physics/static_body.h>
#include <Vital.sandbox/API/physics/character_body.h>
#include <Vital.sandbox/API/physics/animatable_body.h>
#include <Vital.sandbox/API/physics/vehicle_body.h>
#include <Vital.sandbox/API/physics/collision_shape.h>


/////////////////////////////////////////
// Vital: API: Physics Body Spawn Hook //
/////////////////////////////////////////

// TODO: Improve
namespace Vital::Sandbox::API {
    struct Physics_Body_Spawn : vm_module {
        // No Lua-visible scope — this is purely an internal wiring module.
        inline static const std::vector<std::string> base_scope = {};

        static void bind(Machine* vm) {
            // Wire the single global physics body spawn callback.
            // Called once at sandbox init (after all body APIs are registered).
            // On the client, _spawn_entity fires this for every remotely spawned
            // body so Lua can attach collision shapes / wheels in its own handler.

            Vital::Engine::on_physics_body_spawned_callback = [vm](
                Vital::Engine::ISyncable* entity,
                Vital::Engine::PhysicsSubType sub_type,
                bool remote)
            {
                switch (sub_type) {
                    case Vital::Engine::PhysicsSubType::Rigid: {
                        auto* typed = static_cast<Vital::Engine::Rigid_Body*>(entity);
                        {
                            std::lock_guard<std::mutex> lock(Rigid_Body::registry.mutex);
                            for (auto& [id, inst] : Rigid_Body::registry.buffer)
                                if (inst->body == typed) return; // already tracked
                        }
                        auto instance = Rigid_Body::Instance::init(vm, remote);
                        instance->body = typed;
                        instance->store(true);
                        break;
                    }
                    case Vital::Engine::PhysicsSubType::Static: {
                        auto* typed = static_cast<Vital::Engine::Static_Body*>(entity);
                        {
                            std::lock_guard<std::mutex> lock(Static_Body::registry.mutex);
                            for (auto& [id, inst] : Static_Body::registry.buffer)
                                if (inst->body == typed) return;
                        }
                        auto instance = Static_Body::Instance::init(vm, remote);
                        instance->body = typed;
                        instance->store(true);
                        break;
                    }
                    case Vital::Engine::PhysicsSubType::Character: {
                        auto* typed = static_cast<Vital::Engine::Character_Body*>(entity);
                        {
                            std::lock_guard<std::mutex> lock(Character_Body::registry.mutex);
                            for (auto& [id, inst] : Character_Body::registry.buffer)
                                if (inst->body == typed) return;
                        }
                        auto instance = Character_Body::Instance::init(vm, remote);
                        instance->body = typed;
                        instance->store(true);
                        break;
                    }
                    case Vital::Engine::PhysicsSubType::Animatable: {
                        auto* typed = static_cast<Vital::Engine::Animatable_Body*>(entity);
                        {
                            std::lock_guard<std::mutex> lock(Animatable_Body::registry.mutex);
                            for (auto& [id, inst] : Animatable_Body::registry.buffer)
                                if (inst->body == typed) return;
                        }
                        auto instance = Animatable_Body::Instance::init(vm, remote);
                        instance->body = typed;
                        instance->store(true);
                        break;
                    }
                    case Vital::Engine::PhysicsSubType::Vehicle: {
                        auto* typed = static_cast<Vital::Engine::Vehicle_Body*>(entity);
                        {
                            std::lock_guard<std::mutex> lock(Vehicle_Body::registry.mutex);
                            for (auto& [id, inst] : Vehicle_Body::registry.buffer)
                                if (inst->body == typed) return;
                        }
                        auto instance = Vehicle_Body::Instance::init(vm, remote);
                        instance->body = typed;
                        instance->store(true);
                        break;
                    }
                }
            };

            // Wire the single global physics body destroy callback.
            // Fired from PhysicsBodyBase::_notify_predelete_sync() (NOTIFICATION_PREDELETE)
            // for every teardown path of every subtype — not just Lua-initiated
            // ->destroy() calls. Drops the Lua-facing Instance (fires entity:destroyed
            // via erase_unlocked) and nulls its body pointer so nothing can touch the
            // about-to-be-freed object afterward. Mirrors Model::on_destroyed_callback.
            //
            // NOTE: unlike Instance::clean(), this must NOT call body->destroy() again —
            // we're already inside the object's own destruction (PREDELETE), so that
            // would either be a no-op re-entrant call or a double-free depending on the
            // Godot build. Only registry bookkeeping happens here.
            Vital::Engine::on_physics_body_destroyed_callback = [](
                Vital::Engine::ISyncable* entity,
                Vital::Engine::PhysicsSubType sub_type)
            {
                switch (sub_type) {
                    case Vital::Engine::PhysicsSubType::Rigid: {
                        auto* typed = static_cast<Vital::Engine::Rigid_Body*>(entity);
                        std::lock_guard<std::mutex> lock(Rigid_Body::registry.mutex);
                        for (auto it = Rigid_Body::registry.buffer.begin(); it != Rigid_Body::registry.buffer.end();) {
                            auto& instance = it->second;
                            if (instance->body != typed) { ++it; continue; }
                            ++it;
                            Rigid_Body::Instance::erase_unlocked(instance);
                            // Defer the teardown (same reasoning as erase_unlocked's own
                            // deferred buffer-erase): release_userdata_ptr() nulls
                            // instance->userdata, which push_self() also checks. Running
                            // it synchronously here would beat a deferred entity:destroyed
                            // dispatch to the punch, just like the registry erase did.
                            // Queuing it keeps it strictly after that dispatch.
                            Vital::Engine::Core::get_singleton() -> execute([instance]() {
                                instance->body = nullptr;
                                Rigid_Body::Instance::release(instance);
                            });
                        }
                        break;
                    }
                    case Vital::Engine::PhysicsSubType::Static: {
                        auto* typed = static_cast<Vital::Engine::Static_Body*>(entity);
                        std::lock_guard<std::mutex> lock(Static_Body::registry.mutex);
                        for (auto it = Static_Body::registry.buffer.begin(); it != Static_Body::registry.buffer.end();) {
                            auto& instance = it->second;
                            if (instance->body != typed) { ++it; continue; }
                            ++it;
                            Static_Body::Instance::erase_unlocked(instance);
                            // Defer the teardown (same reasoning as erase_unlocked's own
                            // deferred buffer-erase): release_userdata_ptr() nulls
                            // instance->userdata, which push_self() also checks. Running
                            // it synchronously here would beat a deferred entity:destroyed
                            // dispatch to the punch, just like the registry erase did.
                            // Queuing it keeps it strictly after that dispatch.
                            Vital::Engine::Core::get_singleton() -> execute([instance]() {
                                instance->body = nullptr;
                                Static_Body::Instance::release(instance);
                            });
                        }
                        break;
                    }
                    case Vital::Engine::PhysicsSubType::Character: {
                        auto* typed = static_cast<Vital::Engine::Character_Body*>(entity);
                        std::lock_guard<std::mutex> lock(Character_Body::registry.mutex);
                        for (auto it = Character_Body::registry.buffer.begin(); it != Character_Body::registry.buffer.end();) {
                            auto& instance = it->second;
                            if (instance->body != typed) { ++it; continue; }
                            ++it;
                            Character_Body::Instance::erase_unlocked(instance);
                            // Defer the teardown (same reasoning as erase_unlocked's own
                            // deferred buffer-erase): release_userdata_ptr() nulls
                            // instance->userdata, which push_self() also checks. Running
                            // it synchronously here would beat a deferred entity:destroyed
                            // dispatch to the punch, just like the registry erase did.
                            // Queuing it keeps it strictly after that dispatch.
                            Vital::Engine::Core::get_singleton() -> execute([instance]() {
                                instance->body = nullptr;
                                Character_Body::Instance::release(instance);
                            });
                        }
                        break;
                    }
                    case Vital::Engine::PhysicsSubType::Animatable: {
                        auto* typed = static_cast<Vital::Engine::Animatable_Body*>(entity);
                        std::lock_guard<std::mutex> lock(Animatable_Body::registry.mutex);
                        for (auto it = Animatable_Body::registry.buffer.begin(); it != Animatable_Body::registry.buffer.end();) {
                            auto& instance = it->second;
                            if (instance->body != typed) { ++it; continue; }
                            ++it;
                            Animatable_Body::Instance::erase_unlocked(instance);
                            // Defer the teardown (same reasoning as erase_unlocked's own
                            // deferred buffer-erase): release_userdata_ptr() nulls
                            // instance->userdata, which push_self() also checks. Running
                            // it synchronously here would beat a deferred entity:destroyed
                            // dispatch to the punch, just like the registry erase did.
                            // Queuing it keeps it strictly after that dispatch.
                            Vital::Engine::Core::get_singleton() -> execute([instance]() {
                                instance->body = nullptr;
                                Animatable_Body::Instance::release(instance);
                            });
                        }
                        break;
                    }
                    case Vital::Engine::PhysicsSubType::Vehicle: {
                        auto* typed = static_cast<Vital::Engine::Vehicle_Body*>(entity);
                        std::lock_guard<std::mutex> lock(Vehicle_Body::registry.mutex);
                        for (auto it = Vehicle_Body::registry.buffer.begin(); it != Vehicle_Body::registry.buffer.end();) {
                            auto& instance = it->second;
                            if (instance->body != typed) { ++it; continue; }
                            ++it;
                            Vehicle_Body::Instance::erase_unlocked(instance);
                            // Defer the teardown (same reasoning as erase_unlocked's own
                            // deferred buffer-erase): release_userdata_ptr() nulls
                            // instance->userdata, which push_self() also checks. Running
                            // it synchronously here would beat a deferred entity:destroyed
                            // dispatch to the punch, just like the registry erase did.
                            // Queuing it keeps it strictly after that dispatch.
                            Vital::Engine::Core::get_singleton() -> execute([instance]() {
                                instance->body = nullptr;
                                Vehicle_Body::Instance::release(instance);
                            });
                        }
                        break;
                    }
                }
            };

            // Wire the Collision_Shape spawn callback.
            // Fired right after Network::_sync_shape() creates a new Collision_Shape
            // node on a remote-synced physics body (client only). Hydrates a
            // Lua-facing Instance for it — mirrors the physics-body spawn hydration
            // above — so entity:created fires and physics.collision_shape.set_debug_all
            // can find it like any locally-created shape.
            Vital::Engine::Collision_Shape::on_spawned_callback = [vm](
                Vital::Engine::Collision_Shape* node)
            {
                {
                    std::lock_guard<std::mutex> lock(Collision_Shape::registry.mutex);
                    for (auto& [id, inst] : Collision_Shape::registry.buffer)
                        if (inst->body == node) return; // already tracked
                }
                auto instance = Collision_Shape::Instance::init(vm, true);
                instance->body = node;
                instance->store(true);
                if (Collision_Shape::default_debug_enabled) instance->set_debug_visible(true);
            };

            // Wire the Collision_Shape destroy callback.
            // Fired on PREDELETE of any Engine::Collision_Shape node — covers both
            // explicit shape:destroy() and the implicit child-free when the parent
            // body is queue_free()'d by Godot. Nulls instance->body so no Lua call
            // can touch the freed node, then releases the Instance.
            Vital::Engine::Collision_Shape::on_destroyed_callback = [](
                Vital::Engine::Collision_Shape* node)
            {
                std::lock_guard<std::mutex> lock(Collision_Shape::registry.mutex);
                for (auto it = Collision_Shape::registry.buffer.begin();
                          it != Collision_Shape::registry.buffer.end();) {
                    auto& instance = it->second;
                    if (instance->body != node) { ++it; continue; }
                    ++it;
                    Collision_Shape::Instance::erase_unlocked(instance);
                    Vital::Engine::Core::get_singleton()->execute([instance]() {
                        instance->body = nullptr;
                        Collision_Shape::Instance::release(instance);
                    });
                }
            };
        }

        // No per-instance cleanup needed — bodies clean themselves.
        static void clean(const std::string&) {}
    };
}