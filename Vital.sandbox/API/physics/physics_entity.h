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
    struct Physics_Entity {
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
    };
}
