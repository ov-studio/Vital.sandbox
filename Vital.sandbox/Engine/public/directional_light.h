/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: directional_light.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Directional Light Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


///////////////////////////////////////
// Vital: Engine: Directional_Light //
///////////////////////////////////////

namespace Vital::Engine {
    class Directional_Light : public godot::DirectionalLight3D {
        GDCLASS(Directional_Light, godot::DirectionalLight3D)
        private:
            // Instantiators //
            Directional_Light() = default;
            ~Directional_Light() override = default;
            static void _bind_methods() {}
        public:
            // Managers //
            static Directional_Light* create();
            void destroy();
    };
}
#endif