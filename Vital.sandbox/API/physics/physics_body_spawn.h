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

        static void bind(Machine* vm) {
            Vital::Engine::on_spawned_callback = [](Vital::Engine::ISyncable* entity, Vital::Engine::PhysicsType sub_type, bool remote) {
                if (!entity) return;
                // Defer Lua Instance::store / entity:created out of _spawn_entity.
                // Synchronous store during rapid resource restart nests Lua pcall
                // inside network RPC handling and corrupts the VM heap.
                godot::ObjectID oid;
                switch (sub_type) {
                    case Vital::Engine::PhysicsType::Rigid:
                        oid = godot::ObjectID(static_cast<Vital::Engine::Rigid_Body*>(entity)->get_instance_id()); break;
                    case Vital::Engine::PhysicsType::Static:
                        oid = godot::ObjectID(static_cast<Vital::Engine::Static_Body*>(entity)->get_instance_id()); break;
                    case Vital::Engine::PhysicsType::Character:
                        oid = godot::ObjectID(static_cast<Vital::Engine::Character_Body*>(entity)->get_instance_id()); break;
                    case Vital::Engine::PhysicsType::Animatable:
                        oid = godot::ObjectID(static_cast<Vital::Engine::Animatable_Body*>(entity)->get_instance_id()); break;
                    case Vital::Engine::PhysicsType::Vehicle:
                        oid = godot::ObjectID(static_cast<Vital::Engine::Vehicle_Body*>(entity)->get_instance_id()); break;
                    default: return;
                }
                Vital::Engine::Core::get_singleton()->enqueue([oid, sub_type, remote]() {
                    godot::Object* obj = godot::ObjectDB::get_instance(oid);
                    if (!obj) return;
                    switch (sub_type) {
                        case Vital::Engine::PhysicsType::Rigid: {
                            auto* typed = godot::Object::cast_to<Vital::Engine::Rigid_Body>(obj);
                            if (!typed) return;
                            {
                                std::lock_guard<std::mutex> lock(Rigid_Body::registry.mutex);
                                for (auto& [id, inst] : Rigid_Body::registry.buffer)
                                    if (inst->body == typed) return;
                            }
                            auto instance = Rigid_Body::Instance::init(nullptr, remote);
                            instance->body = typed;
                            instance->store(false);
                            break;
                        }
                        case Vital::Engine::PhysicsType::Static: {
                            auto* typed = godot::Object::cast_to<Vital::Engine::Static_Body>(obj);
                            if (!typed) return;
                            {
                                std::lock_guard<std::mutex> lock(Static_Body::registry.mutex);
                                for (auto& [id, inst] : Static_Body::registry.buffer)
                                    if (inst->body == typed) return;
                            }
                            auto instance = Static_Body::Instance::init(nullptr, remote);
                            instance->body = typed;
                            instance->store(false);
                            break;
                        }
                        case Vital::Engine::PhysicsType::Character: {
                            auto* typed = godot::Object::cast_to<Vital::Engine::Character_Body>(obj);
                            if (!typed) return;
                            {
                                std::lock_guard<std::mutex> lock(Character_Body::registry.mutex);
                                for (auto& [id, inst] : Character_Body::registry.buffer)
                                    if (inst->body == typed) return;
                            }
                            auto instance = Character_Body::Instance::init(nullptr, remote);
                            instance->body = typed;
                            instance->store(false);
                            break;
                        }
                        case Vital::Engine::PhysicsType::Animatable: {
                            auto* typed = godot::Object::cast_to<Vital::Engine::Animatable_Body>(obj);
                            if (!typed) return;
                            {
                                std::lock_guard<std::mutex> lock(Animatable_Body::registry.mutex);
                                for (auto& [id, inst] : Animatable_Body::registry.buffer)
                                    if (inst->body == typed) return;
                            }
                            auto instance = Animatable_Body::Instance::init(nullptr, remote);
                            instance->body = typed;
                            instance->store(false);
                            break;
                        }
                        case Vital::Engine::PhysicsType::Vehicle: {
                            auto* typed = godot::Object::cast_to<Vital::Engine::Vehicle_Body>(obj);
                            if (!typed) return;
                            {
                                std::lock_guard<std::mutex> lock(Vehicle_Body::registry.mutex);
                                for (auto& [id, inst] : Vehicle_Body::registry.buffer)
                                    if (inst->body == typed) return;
                            }
                            auto instance = Vehicle_Body::Instance::init(nullptr, remote);
                            instance->body = typed;
                            instance->store(false);
                            break;
                        }
                        default: break;
                    }
                });
            };

            Vital::Engine::on_destroyed_callback = [](
                Vital::Engine::ISyncable* entity,
                Vital::Engine::PhysicsType sub_type)
            {
                switch (sub_type) {
                    case Vital::Engine::PhysicsType::Rigid: {
                        auto* typed = static_cast<Vital::Engine::Rigid_Body*>(entity);
                        std::lock_guard<std::mutex> lock(Rigid_Body::registry.mutex);
                        for (auto it = Rigid_Body::registry.buffer.begin(); it != Rigid_Body::registry.buffer.end();) {
                            auto& instance = it->second;
                            if (instance->body != typed) { ++it; continue; }
                            ++it;
                            Rigid_Body::Instance::erase_unlocked(instance);
                            Vital::Engine::Core::get_singleton() -> execute([instance]() {
                                instance->body = nullptr;
                                Rigid_Body::Instance::release(instance);
                            });
                        }
                        break;
                    }
                    case Vital::Engine::PhysicsType::Static: {
                        auto* typed = static_cast<Vital::Engine::Static_Body*>(entity);
                        std::lock_guard<std::mutex> lock(Static_Body::registry.mutex);
                        for (auto it = Static_Body::registry.buffer.begin(); it != Static_Body::registry.buffer.end();) {
                            auto& instance = it->second;
                            if (instance->body != typed) { ++it; continue; }
                            ++it;
                            Static_Body::Instance::erase_unlocked(instance);
                            Vital::Engine::Core::get_singleton() -> execute([instance]() {
                                instance->body = nullptr;
                                Static_Body::Instance::release(instance);
                            });
                        }
                        break;
                    }
                    case Vital::Engine::PhysicsType::Character: {
                        auto* typed = static_cast<Vital::Engine::Character_Body*>(entity);
                        std::lock_guard<std::mutex> lock(Character_Body::registry.mutex);
                        for (auto it = Character_Body::registry.buffer.begin(); it != Character_Body::registry.buffer.end();) {
                            auto& instance = it->second;
                            if (instance->body != typed) { ++it; continue; }
                            ++it;
                            Character_Body::Instance::erase_unlocked(instance);
                            Vital::Engine::Core::get_singleton() -> execute([instance]() {
                                instance->body = nullptr;
                                Character_Body::Instance::release(instance);
                            });
                        }
                        break;
                    }
                    case Vital::Engine::PhysicsType::Animatable: {
                        auto* typed = static_cast<Vital::Engine::Animatable_Body*>(entity);
                        std::lock_guard<std::mutex> lock(Animatable_Body::registry.mutex);
                        for (auto it = Animatable_Body::registry.buffer.begin(); it != Animatable_Body::registry.buffer.end();) {
                            auto& instance = it->second;
                            if (instance->body != typed) { ++it; continue; }
                            ++it;
                            Animatable_Body::Instance::erase_unlocked(instance);
                            Vital::Engine::Core::get_singleton() -> execute([instance]() {
                                instance->body = nullptr;
                                Animatable_Body::Instance::release(instance);
                            });
                        }
                        break;
                    }
                    case Vital::Engine::PhysicsType::Vehicle: {
                        auto* typed = static_cast<Vital::Engine::Vehicle_Body*>(entity);
                        std::lock_guard<std::mutex> lock(Vehicle_Body::registry.mutex);
                        for (auto it = Vehicle_Body::registry.buffer.begin(); it != Vehicle_Body::registry.buffer.end();) {
                            auto& instance = it->second;
                            if (instance->body != typed) { ++it; continue; }
                            ++it;
                            Vehicle_Body::Instance::erase_unlocked(instance);
                            Vital::Engine::Core::get_singleton() -> execute([instance]() {
                                instance->body = nullptr;
                                Vehicle_Body::Instance::release(instance);
                            });
                        }
                        break;
                    }
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