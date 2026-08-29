/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: static_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Static Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/physics_body.h>


/////////////////////////////////
// Vital: Engine: Static_Body //
/////////////////////////////////

namespace Vital::Engine {
    class Static_Body : public godot::StaticBody3D {
        GDCLASS(Static_Body, godot::StaticBody3D)
        private:
            // Instantiators //
            Static_Body() = default;
            ~Static_Body() override = default;
            static void _bind_methods() {}
        public:
            // Managers //
            static Static_Body* create();
            void destroy();
    };
}
