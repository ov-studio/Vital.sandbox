/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: rigid_body.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rigid Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/rigid_body.h>


/////////////////////////////////
// Vital: Engine: Rigid_Body //
/////////////////////////////////

namespace Vital::Engine {
    // Managers //
    Rigid_Body* Rigid_Body::create() {
        auto body = memnew(Rigid_Body);
        Engine::Core::get_singleton() -> add_child(body);
        return body;
    }

    void Rigid_Body::destroy() {
        queue_free();
    }
}
#endif
