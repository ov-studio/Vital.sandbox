/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: entity.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Entity Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Vital/tool.h>


///////////////////////////
// Vital: Engine: Entity //
///////////////////////////

namespace Vital::Engine {
    // Shared base for every scriptable engine entity. Inherit it *after* the Godot base class.
    template<typename Derived>
    struct Entity {
        protected:
            Entity() = default;
            ~Entity() {
                Tool::Event::emit("entity:unspawn", Tool::Stack({static_cast<Derived*>(this)}));
            }
    };
}
