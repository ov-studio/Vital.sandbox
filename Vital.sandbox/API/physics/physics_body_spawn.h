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


/////////////////////////////////////////
// Vital: API: Physics Body Spawn Hook //
/////////////////////////////////////////

namespace Vital::Sandbox::API {
    struct Physics_Body_Spawn : vm_module {
        // No Lua-visible scope — this is purely an internal wiring module.
        inline static const std::vector<std::string> base_scope = {};

        static void bind(Machine* vm) {
            // Wire the single global physics body spawn callback.
            // Called once at sandbox init (after all body APIs are registered).
            // On the client, _spawn_entity fires this for every remotely spawned
            // body so Lua can attach collision shapes / wheels in its own handler.
            
            // TODO: on_spawned_callback and on_destroyed_callback need and make it work appropriately... this just triggers lua event? and this file into better somewhere in physics body orr soemthing??
            Engine::on_physics_body_spawned_callback = [vm](
                Engine::ISyncable* entity,
                Engine::PhysicsSubType sub_type,
                bool remote)
            {
                switch (sub_type) {
                    case Engine::PhysicsSubType::Rigid: {
                        auto* typed = static_cast<Engine::Rigid_Body*>(entity);
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
                    case Engine::PhysicsSubType::Static: {
                        auto* typed = static_cast<Engine::Static_Body*>(entity);
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
                    case Engine::PhysicsSubType::Character: {
                        auto* typed = static_cast<Engine::Character_Body*>(entity);
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
                    case Engine::PhysicsSubType::Animatable: {
                        auto* typed = static_cast<Engine::Animatable_Body*>(entity);
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
                    case Engine::PhysicsSubType::Vehicle: {
                        auto* typed = static_cast<Engine::Vehicle_Body*>(entity);
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
        }

        // No per-instance cleanup needed — bodies clean themselves.
        static void clean(const std::string&) {}
    };
}
