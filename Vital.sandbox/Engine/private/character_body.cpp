/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: character_body.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Character Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/character_body.h>


////////////////////////////////////
// Vital: Engine: Character_Body //
////////////////////////////////////

namespace Vital::Engine {
    // Managers //
    Character_Body* Character_Body::create() {
        auto body = memnew(Character_Body);
        Engine::Core::get_singleton() -> add_child(body);
        return body;
    }

    void Character_Body::destroy() {
        queue_free();
    }
}
