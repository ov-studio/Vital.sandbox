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
        #if defined(Vital_SDK_Client)
        if (Vital::System::Discord::get_singleton() -> start()) godot::UtilityFunctions::print("Discord Rich Presence initialized");
        else godot::UtilityFunctions::print("Failed to initialize Discord (Maybe discord is not running?)");
        #endif

        #if defined(Vital_SDK_Client)
        if (!Vital::is_editor()) {
            Vital::Godot::Canvas::get_singleton();
            Vital::Godot::Console::get_singleton();
        }
        #endif
        Vital::Godot::Sandbox::get_singleton() -> ready();
    });

    Vital::Tool::Event::bind("vital.core:free", [](Vital::Tool::Stack arguments) -> void {
        if (!Vital::is_editor()) {
            #if defined(Vital_SDK_Client)
            Vital::Godot::Canvas::free_singleton();
            Vital::Godot::Console::free_singleton();
            #endif
            Vital::Godot::Sandbox::free_singleton();
        }

        #if defined(Vital_SDK_Client)
        if (Vital::System::Discord::get_singleton() -> isConnected()) {
            Vital::System::Discord::get_singleton() -> clearActivity();
            Vital::System::Discord::get_singleton() -> tick();
        }
        #endif
    });


    // Sandbox //
    Vital::Tool::Event::bind("vital.sandbox:ready", [](Vital::Tool::Stack arguments) -> void {
        Vital::Sandbox::API::Network::execute("vital.sandbox:ready");

        #if defined(Vital_SDK_Client)
        Vital::System::Discord::ActivityData activity;
        activity.state = "In Game";
        activity.details = "Playing something..";
        Vital::System::Discord::get_singleton() -> setActivity(activity);
        Vital::System::Discord::get_singleton() -> tick();
        #endif

        // TODO: TESTING
        Vital::Godot::Model::load_model("cube", "ladyforaviril.glb");
        auto cube = Vital::Godot::Model::create_object("cube");
        if (cube != nullptr) {
            godot::UtilityFunctions::print("Spawned cube!");
            cube->set_position({0.0f, 0.0f, 0.0f});

            /*
            auto components = cube->get_components();
            godot::UtilityFunctions::print("Available components: ", (int)components.size());
            for (const auto& path : components) {
                godot::UtilityFunctions::print("  - ", Vital::to_godot_string(path));
            }

            //cube->set_component_visible("ACNH Character Armature/Skeleton3D/Hair 01/Hair 01", false);
            //cube->set_component_visible("ACNH Character Armature/Skeleton3D/Hair_02/Hair_02", false);
            */

            /*
            auto blend_shapes = cube->get_blend_shapes("Hair_bang_R/Skeleton3D/Torso_vest_");
            godot::UtilityFunctions::print("Blend shapes for Torso_vest_: ", (int)blend_shapes.size());
            for (const auto& shape : blend_shapes) {
                godot::UtilityFunctions::print("  - ", Vital::to_godot_string(shape));
            }
            
            //cube->set_blend_shape_value("Hair_bang_R/Skeleton3D/Torso_vest_", "Smile", 1.0f);
            */

            // Play animation on loop
            // First, let's see what animations are available
            auto available_anims = cube->get_animations();
            godot::UtilityFunctions::print("Available animations: ", (int)available_anims.size());
            if (!available_anims.empty()) {
                cube -> play_animation(available_anims[0], true, 1.0f);
                godot::UtilityFunctions::print("Playing animation: ", Vital::to_godot_string(available_anims[0]));
            }

            //tree->set_rotation(0.0f, Math::randf() * 360.0f, 0.0f);
            //props.push_back(tree);
        }
    });

    Vital::Tool::Event::bind("vital.sandbox:process", [](Vital::Tool::Stack arguments) -> void {
        Vital::Sandbox::API::Network::execute("vital.sandbox:process", arguments.object["delta"].as<double>());
        #if defined(Vital_SDK_Client)
        Vital::System::Discord::get_singleton() -> tick();
        #endif
    });

    Vital::Tool::Event::bind("vital.sandbox:draw", [](Vital::Tool::Stack arguments) -> void {
        Vital::Sandbox::API::Network::execute("vital.sandbox:draw");
    });

    Vital::Tool::Event::bind("vital.sandbox:key_input", [](Vital::Tool::Stack arguments) -> void {
        Vital::Sandbox::API::Network::execute("vital.sandbox:key_input", arguments.object["keycode"].as<std::string>(), arguments.object["state"].as<bool>());
    });

    Vital::Tool::Event::bind("vital.sandbox:mouse_move", [](Vital::Tool::Stack arguments) -> void {
        Vital::Sandbox::API::Network::execute("vital.sandbox:mouse_move", arguments.object["x"].as<float>(), arguments.object["y"].as<float>());
    });

    Vital::Tool::Event::bind("vital.sandbox:console_input", [](Vital::Tool::Stack arguments) -> void {
        Vital::Sandbox::API::Network::execute("vital.sandbox:console_input", arguments.object["command"].as<std::string>(), arguments.object["parameters"].as<std::vector<std::string>>());
    });
}