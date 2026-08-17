/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: area_light.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Area Light Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


////////////////////////////////
// Vital: Engine: Area_Light //
////////////////////////////////

namespace Vital::Engine {
    class Area_Light : public godot::AreaLight3D {
        GDCLASS(Area_Light, godot::AreaLight3D)
        private:
            // Instantiators //
            Area_Light() = default;
            ~Area_Light() override = default;
            static void _bind_methods() {}
        public:
            // Managers //
            static Area_Light* create();
            void destroy();
    };
}
#endif
