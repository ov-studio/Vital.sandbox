/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: static_body.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Static Body Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/static_body.h>


/////////////////////////////////
// Vital: Engine: Static_Body //
/////////////////////////////////

namespace Vital::Engine {
    // Managers //
    Static_Body* Static_Body::create() {
        auto body = memnew(Static_Body);
        Engine::Core::get_singleton() -> add_child(body);
        return body;
    }

    void Static_Body::destroy() {
        queue_free();
    }
}
#endif
