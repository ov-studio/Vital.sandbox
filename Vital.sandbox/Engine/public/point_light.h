/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: point_light.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Point Light Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


/////////////////////////////////
// Vital: Engine: Point_Light //
/////////////////////////////////

namespace Vital::Engine {
    class Point_Light : public godot::OmniLight3D {
        GDCLASS(Point_Light, godot::OmniLight3D)
        private:
            // Instantiators //
            Point_Light() = default;
            ~Point_Light() override = default;
            static void _bind_methods() {}
        public:
            // Managers //
            static Point_Light* create();
            void destroy();
    };
}
#endif