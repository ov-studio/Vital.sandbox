/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: monitor.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Monitor Bridge
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Vital/sandbox.h>


////////////////////////////
// Vital: Engine: Monitor //
////////////////////////////

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