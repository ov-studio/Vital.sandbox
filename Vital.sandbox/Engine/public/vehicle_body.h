/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: vehicle_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vehicle Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/physics_body.h>


//////////////////////////////////
// Vital: Engine: Vehicle_Body //
//////////////////////////////////

namespace Vital::Manager { 
    class Network; 
}

namespace Vital::Engine {
    class Vehicle_Body : public Physics_Body<godot::VehicleBody3D> {
        GDCLASS(Vehicle_Body, godot::VehicleBody3D)
        friend class Manager::Network;
        friend class Network;
        private:
            // Instantiators //
            Vehicle_Body() = default;
            ~Vehicle_Body() override = default;
            static void _bind_methods() {}
        public:
            // Hooks //
            void _ready() override;
            void _process(double delta) override;

            
            // Managers //
            static Vehicle_Body* create(int authority_peer = 0);
            void destroy();


            // Getters //
            PhysicsType get_physics_type() const override;
    };
}
