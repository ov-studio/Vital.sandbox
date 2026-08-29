/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: area.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Area Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>


//////////////////////////
// Vital: Engine: Area //
//////////////////////////

namespace Vital::Engine {
    class Area : public godot::Area3D {
        GDCLASS(Area, godot::Area3D)
        private:
            // Instantiators //
            Area() = default;
            ~Area() override = default;
            static void _bind_methods() {}
        public:
            // Managers //
            static Area* create();
            void destroy();
    };
}
