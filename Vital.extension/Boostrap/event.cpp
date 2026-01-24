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
    // Core //
    Vital::Tool::Event::bind("vital.core:ready", [](Vital::Tool::Stack arguments) -> void {
        if (!Vital::is_editor()) {
            #if defined(Vital_SDK_Client)
            Vital::Godot::Core::get_environment();
            Vital::Godot::Canvas::get_singleton();
            Vital::Godot::Console::get_singleton();
            #endif
            Vital::Godot::Sandbox::get_singleton() -> ready();
        }

        #if defined(Vital_SDK_Client)
        if (Vital::System::Discord::start()) {
            godot::UtilityFunctions::print("Discord Rich Presence initialized");
            if (Vital::System::Discord::setActivity("In Main Menu", "Thinking about what to do..")) godot::UtilityFunctions::print("Discord Rich Presence activity set");
            else godot::UtilityFunctions::print("Failed to set Discord Rich Presence activity");
        }
        else godot::UtilityFunctions::print("Failed to initialize Discord (Maybe discord is not running?)");
        #endif
    });
    
    Vital::Tool::Event::bind("vital.core:free", [](Vital::Tool::Stack arguments) -> void {
        if (!Vital::is_editor()) {
            #if defined(Vital_SDK_Client)
            Vital::Godot::Core::free_environment();
            Vital::Godot::Canvas::free_singleton();
            Vital::Godot::Console::free_singleton();
            #endif
            Vital::Godot::Sandbox::free_singleton();
        }
        
        #if defined(Vital_SDK_Client)
        Vital::System::Discord::stop();
        #endif
    });


    // Sandbox //
    Vital::Tool::Event::bind("vital.sandbox:ready", [](Vital::Tool::Stack arguments) -> void {

    });

    Vital::Tool::Event::bind("vital.sandbox:process", [](Vital::Tool::Stack arguments) -> void {

    });

    Vital::Tool::Event::bind("vital.sandbox:draw", [](Vital::Tool::Stack arguments) -> void {

    });

    Vital::Tool::Event::bind("vital.sandbox:input", [](Vital::Tool::Stack arguments) -> void {
        auto keycode = arguments.object["keycode"].as<std::string>();
        auto state = arguments.object["state"].as<bool>();
        godot::UtilityFunctions::print("Key:  ", Vital::to_godot_string(keycode), " ", (state ? "pressed" : "released"));
    });

    Vital::Tool::Event::bind("vital.sandbox:mouse_move", [](Vital::Tool::Stack arguments) -> void {
        auto x = arguments.object["x"].as<float>();
        auto y = arguments.object["y"].as<float>();
        godot::UtilityFunctions::print("Mouse moved: ", x, ", ", y);
    });
}