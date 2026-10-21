/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: vehicle_wheel.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vehicle Wheel Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/vehicle_wheel.h>


///////////////////////////////////
// Vital: Engine: Vehicle_Wheel //
///////////////////////////////////

namespace Vital::Engine {
    // Hooks //
    void Vehicle_Wheel::_notification(int what) {
        if (what == NOTIFICATION_PREDELETE) Tool::Event::emit("entity:unspawned", Tool::Stack({this}));
    }


    // Managers //
    Vehicle_Wheel* Vehicle_Wheel::create(godot::Node3D* body) {
        auto wheel = memnew(Vehicle_Wheel);
        if (body) body -> add_child(wheel);
        else Engine::Core::get_singleton() -> add_child(wheel);
        if (auto vehicle = godot::Object::cast_to<godot::VehicleBody3D>(body)) wheel -> set_brake(vehicle -> get_brake());
        wheel -> sync_inputs();
        Tool::Event::emit("entity:spawned", Tool::Stack({wheel, false}));
        Tool::Event::emit("entity:ready", Tool::Stack({static_cast<godot::Node3D*>(wheel)}));
        return wheel;
    }

    void Vehicle_Wheel::destroy() {
        queue_free();
    }

    void Vehicle_Wheel::refresh() {
        const godot::ObjectID oid(get_instance_id());
        Engine::Core::get_singleton() -> execute([oid]() {
            auto wheel = godot::Object::cast_to<Vehicle_Wheel>(godot::ObjectDB::get_instance(oid));
            if (!wheel || !wheel -> is_inside_tree()) return;
            auto parent = wheel -> get_parent();
            if (!parent) return;
            const int index = wheel -> get_index();
            parent -> remove_child(wheel);
            parent -> add_child(wheel);
            parent -> move_child(wheel, index);
        });
    }

    void Vehicle_Wheel::sync_inputs() {
        auto vehicle = godot::Object::cast_to<godot::VehicleBody3D>(get_parent());
        if (!vehicle) return;
        if (is_used_as_traction()) set_engine_force(vehicle -> get_engine_force());
        if (is_used_as_steering()) set_steering(vehicle -> get_steering());
    }


    // Getters //
    int Vehicle_Wheel::get_wheel_id() const {
        return wheel_id;
    }

    uint32_t Vehicle_Wheel::get_parent_net_id() const {
        return Engine::get_parent_net_id(this);
    }


    // Setters //
    void Vehicle_Wheel::set_wheel_id(int index) {
        wheel_id = index;
    }
}
