/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Boostrap: event.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Event Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Boostrap/index.h>


/////////////
// Events //
/////////////

void initialize_vital_events() {
    Vital::Tool::Event::bind("vital.core:ready", [](Vital::Tool::Stack arguments) -> void {
        #if defined(Vital_SDK_Client)
        Vital::Godot::Core::get_environment();
        Vital::Godot::Canvas::get_singleton();
        Vital::Godot::Console::get_singleton();
        #endif
        Vital::Godot::Sandbox::get_singleton() -> ready();
    });
    
    Vital::Tool::Event::bind("vital.core:free", [](Vital::Tool::Stack arguments) -> void {
        #if defined(Vital_SDK_Client)
        Vital::Godot::Core::free_environment();
        Vital::Godot::Canvas::free_singleton();
        Vital::Godot::Console::free_singleton();
        #endif
        Vital::Godot::Sandbox::free_singleton();
    });
}