/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: character_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Character Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/physics_body.h>


////////////////////////////////////
// Vital: Engine: Character_Body //
////////////////////////////////////

namespace Vital::Manager { 
    class Network; 
}

namespace Vital::Engine {
    class Character_Body : public Physics_Body<godot::CharacterBody3D> {
        GDCLASS(Character_Body, godot::CharacterBody3D)
        friend class Manager::Network;
        friend class Network;
        private:
            // Instantiators //
            Character_Body() = default;
            ~Character_Body() override = default;
            static void _bind_methods() {}
        public:
            // Hooks //
            void _ready() override;
            void _process(double delta) override;

            
            // Managers //
            static Character_Body* create(int authority_peer = 0);
            void destroy();


            // Getters //
            PhysicsType get_physics_type() const override;
    };
}
