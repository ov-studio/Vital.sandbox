/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: monitor.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Monitor Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


/////////////////////////////
// Vital: Engine: Monitor //
/////////////////////////////

namespace Vital::Engine {
    class Monitor : public godot::Object {
        GDCLASS(Monitor, godot::Object)
        protected:
            static void _bind_methods() {
                godot::ClassDB::bind_method(godot::D_METHOD("dispatch", "key"), &Monitor::dispatch);
            }
        public:
            godot::Variant dispatch(const godot::String& key);
    };
}
#endif