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

using namespace godot;
using namespace Vital::Tool;

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


        Stack stack;

        stack.values.emplace_back(42);             // int
        stack.values.emplace_back(3.14f);          // float
        stack.values.emplace_back(2.718);          // double

        stack.named["pi"] = StackValue(3.14159);
        stack.named["answer"] = StackValue(42);

        // -----------------------------
        // Serialize
        // -----------------------------
        std::string data = stack.serialize();
        UtilityFunctions::print("Serialized size: " + Vital::to_godot_string(std::to_string(data.size())) + " bytes");

        // -----------------------------
        // Deserialize
        // -----------------------------
        Stack restored = Stack::deserialize(data);

        int number = restored.values[0].as<int32_t>();
        UtilityFunctions::print(number);
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
    });


    // Sandbox //
    Vital::Tool::Event::bind("vital.sandbox:ready", [](Vital::Tool::Stack arguments) -> void {

    });

    Vital::Tool::Event::bind("vital.sandbox:process", [](Vital::Tool::Stack arguments) -> void {

    });

    Vital::Tool::Event::bind("vital.sandbox:draw", [](Vital::Tool::Stack arguments) -> void {

    });
}