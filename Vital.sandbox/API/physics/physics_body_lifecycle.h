/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: physics: physics_body_lifecycle.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Physics Body Lifecycle APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/API/physics/rigid_body.h>
#include <Vital.sandbox/API/physics/static_body.h>
#include <Vital.sandbox/API/physics/character_body.h>
#include <Vital.sandbox/API/physics/animatable_body.h>
#include <Vital.sandbox/API/physics/vehicle_body.h>


/////////////////////////////////////////
// Vital: API: Physics_Body_Lifecycle //
/////////////////////////////////////////

namespace Vital::Sandbox::API {
    struct Physics_Body_Lifecycle : vm_module {
        inline static const std::vector<std::string> base_scope = {};

        template<typename API_Type, typename Engine_Type>
        static void spawn_body(godot::ObjectID oid, bool remote) {
            Vital::Engine::Core::get_singleton()->enqueue([oid, remote]() {
                godot::Object* obj = godot::ObjectDB::get_instance(oid);
                if (!obj) return;
                auto* typed = godot::Object::cast_to<Engine_Type>(obj);
                if (!typed) return;
                if (API_Type::Instance::find_by_ptr(typed)) return;
                auto instance = API_Type::Instance::init(nullptr, remote);
                instance->body = typed;
                instance->store(false);
            });
        }

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

        static void init(Machine* vm) {
            static Tool::Event::event_id spawned_binding = 0;
            static Tool::Event::event_id destroyed_binding = 0;
            if (spawned_binding) Tool::Event::unbind("entity:spawned", spawned_binding);
            if (destroyed_binding) Tool::Event::unbind("entity:unspawned", destroyed_binding);

            spawned_binding = Tool::Event::bind("entity:spawned", [](Tool::Stack args) {
                if (args.array.size() < 3) return;
                auto* entity = args.array[0].as<Vital::Engine::ISyncable*>();
                if (!entity) return;
                auto sub_type = (Vital::Engine::PhysicsType)args.array[1].as<int32_t>();
                bool remote = args.array[2].as<bool>();
                switch (sub_type) {
                    case Vital::Engine::PhysicsType::Rigid:
                        spawn_body<Rigid_Body, Vital::Engine::Rigid_Body>(godot::ObjectID(static_cast<Vital::Engine::Rigid_Body*>(entity)->get_instance_id()), remote);
                        break;
                    case Vital::Engine::PhysicsType::Static:
                        spawn_body<Static_Body, Vital::Engine::Static_Body>(godot::ObjectID(static_cast<Vital::Engine::Static_Body*>(entity)->get_instance_id()), remote);
                        break;
                    case Vital::Engine::PhysicsType::Character:
                        spawn_body<Character_Body, Vital::Engine::Character_Body>(godot::ObjectID(static_cast<Vital::Engine::Character_Body*>(entity)->get_instance_id()), remote);
                        break;
                    case Vital::Engine::PhysicsType::Animatable:
                        spawn_body<Animatable_Body, Vital::Engine::Animatable_Body>(godot::ObjectID(static_cast<Vital::Engine::Animatable_Body*>(entity)->get_instance_id()), remote);
                        break;
                    case Vital::Engine::PhysicsType::Vehicle:
                        spawn_body<Vehicle_Body, Vital::Engine::Vehicle_Body>(godot::ObjectID(static_cast<Vital::Engine::Vehicle_Body*>(entity)->get_instance_id()), remote);
                        break;
                    default: break;
                }
            });

            destroyed_binding = Tool::Event::bind("entity:unspawned", [](Tool::Stack args) {
                if (args.array.size() < 2) return;
                auto* entity = args.array[0].as<Vital::Engine::ISyncable*>();
                if (!entity) return;
                auto sub_type = (Vital::Engine::PhysicsType)args.array[1].as<int32_t>();
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
                    default: break;
                }
            });
        }

        static void bind(Machine* vm) {}

        static void clean(const std::string&) {}
    };
}
