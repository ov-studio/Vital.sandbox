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
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


/////////////////////////////////////
// Vital: Engine: Character_Body //
/////////////////////////////////////

namespace Vital::Engine {
    class Character_Body : public godot::CharacterBody3D {
        GDCLASS(Character_Body, godot::CharacterBody3D)
        private:
            // Instantiators //
            Character_Body() = default;
            ~Character_Body() override = default;
            static void _bind_methods() {}
        public:
            // Managers //
            static Character_Body* create();
            void destroy();
    };
}
#endif
