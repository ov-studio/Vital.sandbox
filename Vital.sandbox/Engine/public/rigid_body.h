/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: rigid_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rigid Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


////////////////////////////////
// Vital: Engine: Rigid_Body //
////////////////////////////////

namespace Vital::Engine {
    class Rigid_Body : public godot::RigidBody3D {
        GDCLASS(Rigid_Body, godot::RigidBody3D)
        private:
            // Instantiators //
            Rigid_Body() = default;
            ~Rigid_Body() override = default;
            static void _bind_methods() {}
        public:
            // Managers //
            static Rigid_Body* create();
            void destroy();
    };
}
#endif
