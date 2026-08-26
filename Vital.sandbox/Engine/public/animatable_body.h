/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: animatable_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Animatable Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


/////////////////////////////////////
// Vital: Engine: Animatable_Body //
/////////////////////////////////////

namespace Vital::Engine {
    class Animatable_Body : public godot::AnimatableBody3D {
        GDCLASS(Animatable_Body, godot::AnimatableBody3D)
        private:
            // Instantiators //
            Animatable_Body() = default;
            ~Animatable_Body() override = default;
            static void _bind_methods() {}
        public:
            // Managers //
            static Animatable_Body* create();
            void destroy();
    };
}
#endif
