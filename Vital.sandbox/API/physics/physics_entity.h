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
#include <Vital.sandbox/API/base/node_3d.h>
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
        
        template<typename T>
        struct Body_Tag { using API_Type = T; };

        template<typename Fn>
        static void dispatch(Vital::Engine::PhysicsType type, Fn fn) {
            switch (type) {
                case Vital::Engine::PhysicsType::Rigid:      fn(Body_Tag<Rigid_Body>{});      break;
                case Vital::Engine::PhysicsType::Static:     fn(Body_Tag<Static_Body>{});     break;
                case Vital::Engine::PhysicsType::Character:  fn(Body_Tag<Character_Body>{});  break;
                case Vital::Engine::PhysicsType::Animatable: fn(Body_Tag<Animatable_Body>{}); break;
                case Vital::Engine::PhysicsType::Vehicle:    fn(Body_Tag<Vehicle_Body>{});    break;
                default: break;
            }
        }

        static void init(Machine* vm) {
            static Tool::Event::Handle spawned_binding;
            spawned_binding.bind("entity:spawn", [](Tool::Stack args) {
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
                    default: 
                        break;
                }
            });

            static Tool::Event::Handle unspawned_binding;
            unspawned_binding.bind("entity:unspawn", [](Tool::Stack args) {
                if (args.array.size() < 2) return;
                auto entity = args.array[0].as<Vital::Engine::ISyncable*>();
                if (!entity) return;

                auto sub_type = (Vital::Engine::PhysicsType)args.array[1].as<int32_t>();
                dispatch(sub_type, [&](auto tag) {
                    using API_Type = typename decltype(tag)::API_Type;
                    API_Type::Instance::destroy_by_ptr(static_cast<typename API_Type::base_class*>(entity), [](std::shared_ptr<typename API_Type::Instance> instance) {
                        instance -> body = nullptr;
                    });
                });
            });
        }
    };
}
