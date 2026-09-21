/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: physics: physics_entity.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Physics Entity APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/API/physics/rigid_body.h>
#include <Vital.sandbox/API/physics/static_body.h>
#include <Vital.sandbox/API/physics/character_body.h>
#include <Vital.sandbox/API/physics/animatable_body.h>
#include <Vital.sandbox/API/physics/vehicle_body.h>


/////////////////////////////////
// Vital: API: Physics_Entity //
/////////////////////////////////

namespace Vital::Sandbox::API {
    struct Physics_Entity : vm_module {
        inline static const std::vector<std::string> base_scope = {};

        static bool is_body(Machine* vm, int idx) {
            return vm_module::is_userdata<Rigid_Body::Instance>(vm, idx)      ||
                   vm_module::is_userdata<Static_Body::Instance>(vm, idx)     ||
                   vm_module::is_userdata<Character_Body::Instance>(vm, idx)  ||
                   vm_module::is_userdata<Animatable_Body::Instance>(vm, idx) ||
                   vm_module::is_userdata<Vehicle_Body::Instance>(vm, idx);
        }

        static godot::Node3D* resolve_body(Machine* vm, int idx) {
            if (vm_module::is_userdata<Rigid_Body::Instance>(vm, idx))      return vm_module::get_userdata_object<Rigid_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Static_Body::Instance>(vm, idx))     return vm_module::get_userdata_object<Static_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Character_Body::Instance>(vm, idx))  return vm_module::get_userdata_object<Character_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Animatable_Body::Instance>(vm, idx)) return vm_module::get_userdata_object<Animatable_Body::Instance>(vm, idx) -> get_node();
            if (vm_module::is_userdata<Vehicle_Body::Instance>(vm, idx))    return vm_module::get_userdata_object<Vehicle_Body::Instance>(vm, idx) -> get_node();
            return nullptr;
        }
        
        template<typename API_Type, typename Engine_Type>
        static void spawn_body(godot::ObjectID oid, bool remote) {
            Vital::Engine::Core::get_singleton() -> enqueue([oid, remote]() {
                godot::Object* obj = godot::ObjectDB::get_instance(oid);
                if (!obj) return;
                auto typed = godot::Object::cast_to<Engine_Type>(obj);
                if (!typed) return;
                if (API_Type::Instance::find_by_ptr(typed)) return;

                auto instance = API_Type::Instance::init(nullptr, remote);
                instance -> body = typed;
                instance -> store(false);
            });
        }

        template<typename API_Type, typename Engine_Type>
        static void destroy_body(Vital::Engine::ISyncable* entity) {
            auto typed = static_cast<Engine_Type*>(entity);
            API_Type::Instance::destroy_by_ptr(typed, [](std::shared_ptr<typename API_Type::Instance> instance) {
                instance -> body = nullptr;
            });
        }

        static void init(Machine* vm) {
            static Tool::Event::Handle spawned_binding;
            spawned_binding.bind("entity:spawned", [](Tool::Stack args) {
                if (args.array.size() < 3) return;
                auto entity = args.array[0].as<Vital::Engine::ISyncable*>();
                if (!entity) return;
                
                auto sub_type = (Vital::Engine::PhysicsType)args.array[1].as<int32_t>();
                bool remote = args.array[2].as<bool>();
                switch (sub_type) {
                    case Vital::Engine::PhysicsType::Rigid:
                        spawn_body<Rigid_Body, Vital::Engine::Rigid_Body>(godot::ObjectID(static_cast<Vital::Engine::Rigid_Body*>(entity) -> get_instance_id()), remote);
                        break;
                    case Vital::Engine::PhysicsType::Static:
                        spawn_body<Static_Body, Vital::Engine::Static_Body>(godot::ObjectID(static_cast<Vital::Engine::Static_Body*>(entity) -> get_instance_id()), remote);
                        break;
                    case Vital::Engine::PhysicsType::Character:
                        spawn_body<Character_Body, Vital::Engine::Character_Body>(godot::ObjectID(static_cast<Vital::Engine::Character_Body*>(entity) -> get_instance_id()), remote);
                        break;
                    case Vital::Engine::PhysicsType::Animatable:
                        spawn_body<Animatable_Body, Vital::Engine::Animatable_Body>(godot::ObjectID(static_cast<Vital::Engine::Animatable_Body*>(entity) -> get_instance_id()), remote);
                        break;
                    case Vital::Engine::PhysicsType::Vehicle:
                        spawn_body<Vehicle_Body, Vital::Engine::Vehicle_Body>(godot::ObjectID(static_cast<Vital::Engine::Vehicle_Body*>(entity) -> get_instance_id()), remote);
                        break;
                    default: break;
                }
            });

            static Tool::Event::Handle unspawned_binding;
            unspawned_binding.bind("entity:unspawned", [](Tool::Stack args) {
                if (args.array.size() < 2) return;
                auto entity = args.array[0].as<Vital::Engine::ISyncable*>();
                if (!entity) return;

                auto sub_type = (Vital::Engine::PhysicsType)args.array[1].as<int32_t>();
                switch (sub_type) {
                    case Vital::Engine::PhysicsType::Rigid:
                        destroy_body<Rigid_Body, Vital::Engine::Rigid_Body>(entity); 
                        break;
                    case Vital::Engine::PhysicsType::Static:
                        destroy_body<Static_Body, Vital::Engine::Static_Body>(entity); 
                        break;
                    case Vital::Engine::PhysicsType::Character:
                        destroy_body<Character_Body, Vital::Engine::Character_Body>(entity); 
                        break;
                    case Vital::Engine::PhysicsType::Animatable:
                        destroy_body<Animatable_Body, Vital::Engine::Animatable_Body>(entity); 
                        break;
                    case Vital::Engine::PhysicsType::Vehicle:
                        destroy_body<Vehicle_Body, Vital::Engine::Vehicle_Body>(entity); 
                        break;
                    default: break;
                }
            });
        }
    };
}
