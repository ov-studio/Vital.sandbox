/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: spot_light.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Spot Light Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


////////////////////////////////
// Vital: Engine: Spot_Light //
////////////////////////////////

namespace Vital::Engine {
    class Spot_Light : public godot::SpotLight3D {
        GDCLASS(Spot_Light, godot::SpotLight3D)
        private:
            // Instantiators //
            Spot_Light() = default;
            ~Spot_Light() override = default;
            static void _bind_methods() {}
        public:
            // Managers //
            static Spot_Light* create();
            void destroy();
    };
}
#endif