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
           Physics_Body::_notify_predelete_sync() (NOTIFICATION_PREDELETE)
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
        inline static const std::vector<std::string> base_scope = {};

        // -----------------------------------------------------------------------
        // spawn_body<API_Type, Engine_Type>
        //
        // Defers the Lua Instance::store() / entity:created signal out of the
        // _spawn_entity RPC handler to avoid nesting a Lua pcall inside network
        // RPC handling (which corrupts the VM heap under rapid resource restart).
        //
        // Idempotent: bails early if an Instance already owns this engine node
        // (prevents double-registration on the same object).
        // -----------------------------------------------------------------------
        template<typename API_Type, typename Engine_Type>
        static void spawn_body(godot::ObjectID oid, bool remote) {
            Vital::Engine::Core::get_singleton()->enqueue([oid, remote]() {
                godot::Object* obj = godot::ObjectDB::get_instance(oid);
                if (!obj) return;
                auto* typed = godot::Object::cast_to<Engine_Type>(obj);
                if (!typed) return;
                {
                    std::lock_guard<std::mutex> lock(API_Type::registry.mutex);
                    for (auto& [id, inst] : API_Type::registry.buffer)
                        if (inst->body == typed) return;
                }
                auto instance = API_Type::Instance::init(nullptr, remote);
                instance->body = typed;
                instance->store(false);
            });
        }

        // -----------------------------------------------------------------------
        // destroy_body<API_Type, Engine_Type>
        //
        // Walks the registry under its lock, finds every Instance whose body
        // pointer matches the dying engine node, erases it (emitting
        // entity:destroyed), then schedules a Core::execute() to null the pointer
        // and release Lua references off the physics thread.
        // -----------------------------------------------------------------------
        template<typename API_Type, typename Engine_Type>
        static void destroy_body(Vital::Engine::ISyncable* entity) {
            auto* typed = static_cast<Engine_Type*>(entity);
            std::lock_guard<std::mutex> lock(API_Type::registry.mutex);
            for (auto it = API_Type::registry.buffer.begin(); it != API_Type::registry.buffer.end();) {
                auto& instance = it->second;
                if (instance->body != typed) { ++it; continue; }
                ++it;
                API_Type::Instance::erase_unlocked(instance);
                Vital::Engine::Core::get_singleton()->execute([instance]() {
                    instance->body = nullptr;
                    API_Type::Instance::release(instance);
                });
            }
        }

        static void bind(Machine* vm) {
            Vital::Engine::on_spawned_callback = [](Vital::Engine::ISyncable* entity, Vital::Engine::PhysicsType sub_type, bool remote) {
                if (!entity) return;
                // Capture by ObjectID — raw `this` would be dangling if the body is
                // queue_free()'d before the deferred enqueue drains.
                switch (sub_type) {
                    case Vital::Engine::PhysicsType::Rigid:
                        spawn_body<Rigid_Body, Vital::Engine::Rigid_Body>(
                            godot::ObjectID(static_cast<Vital::Engine::Rigid_Body*>(entity)->get_instance_id()), remote);
                        break;
                    case Vital::Engine::PhysicsType::Static:
                        spawn_body<Static_Body, Vital::Engine::Static_Body>(
                            godot::ObjectID(static_cast<Vital::Engine::Static_Body*>(entity)->get_instance_id()), remote);
                        break;
                    case Vital::Engine::PhysicsType::Character:
                        spawn_body<Character_Body, Vital::Engine::Character_Body>(
                            godot::ObjectID(static_cast<Vital::Engine::Character_Body*>(entity)->get_instance_id()), remote);
                        break;
                    case Vital::Engine::PhysicsType::Animatable:
                        spawn_body<Animatable_Body, Vital::Engine::Animatable_Body>(
                            godot::ObjectID(static_cast<Vital::Engine::Animatable_Body*>(entity)->get_instance_id()), remote);
                        break;
                    case Vital::Engine::PhysicsType::Vehicle:
                        spawn_body<Vehicle_Body, Vital::Engine::Vehicle_Body>(
                            godot::ObjectID(static_cast<Vital::Engine::Vehicle_Body*>(entity)->get_instance_id()), remote);
                        break;
                    default: break;
                }
            };

            Vital::Engine::on_destroyed_callback = [](Vital::Engine::ISyncable* entity, Vital::Engine::PhysicsType sub_type) {
                switch (sub_type) {
                    case Vital::Engine::PhysicsType::Rigid:
                        destroy_body<Rigid_Body, Vital::Engine::Rigid_Body>(entity); break;
                    case Vital::Engine::PhysicsType::Static:
                        destroy_body<Static_Body, Vital::Engine::Static_Body>(entity); break;
                    case Vital::Engine::PhysicsType::Character:
                        destroy_body<Character_Body, Vital::Engine::Character_Body>(entity); break;
                    case Vital::Engine::PhysicsType::Animatable:
                        destroy_body<Animatable_Body, Vital::Engine::Animatable_Body>(entity); break;
                    case Vital::Engine::PhysicsType::Vehicle:
                        destroy_body<Vehicle_Body, Vital::Engine::Vehicle_Body>(entity); break;
                }
            };

            Vital::Engine::Collision_Shape::on_spawned_callback = [](Vital::Engine::Collision_Shape* node) {
                if (!node) return;
                {
                    std::lock_guard<std::mutex> lock(Collision_Shape::registry.mutex);
                    for (auto& [id, inst] : Collision_Shape::registry.buffer)
                        if (inst->body == node) return;
                }
                const godot::ObjectID oid(node->get_instance_id());
                Vital::Engine::Core::get_singleton()->enqueue([oid]() {
                    godot::Object* obj = godot::ObjectDB::get_instance(oid);
                    if (!obj) return;
                    auto* shape = godot::Object::cast_to<Vital::Engine::Collision_Shape>(obj);
                    if (!shape) return;
                    {
                        std::lock_guard<std::mutex> lock(Collision_Shape::registry.mutex);
                        for (auto& [id, inst] : Collision_Shape::registry.buffer)
                            if (inst->body == shape) return;
                    }
                    auto instance = Collision_Shape::Instance::init(nullptr, true);
                    instance->body = shape;
                    instance->store(false);
                });
            };

            Vital::Engine::Collision_Shape::on_destroyed_callback = [](Vital::Engine::Collision_Shape* node) {
                std::lock_guard<std::mutex> lock(Collision_Shape::registry.mutex);
                for (auto it = Collision_Shape::registry.buffer.begin(); it != Collision_Shape::registry.buffer.end();) {
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

        static void clean(const std::string&) {}
    };
}
