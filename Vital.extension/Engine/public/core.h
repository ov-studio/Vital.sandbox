/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: core.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Core Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/index.h>


///////////////////////////
// Vital: Godot: Engine //
///////////////////////////

namespace Vital::Godot {
    class Core;
    class Core : public godot::Node {
        GDCLASS(Core, godot::Node)
        protected:
            inline static Core* singleton = nullptr;
            inline static godot::WorldEnvironment* environment = nullptr;
            static void _bind_methods() {};
        public:
            // Instantiators //
            Core() = default;
            ~Core() override = default;
            void _ready() override;
            void _exit_tree() override;
            void _process(double delta) override;
            void _unhandled_input(godot::Ref<godot::InputEvent> event);


            // Getters //
            static Core* get_singleton();
            static godot::Node* get_root();
            #if defined(Vital_SDK_Client)
            static godot::RenderingServer* get_rendering_server();
            static godot::Ref<godot::Environment> get_environment();
            static void free_environment();
            #endif
    };
}