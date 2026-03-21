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


////////////////////////////
// Vital: Engine: Engine //
////////////////////////////

namespace Vital::Engine {
    class Core : public godot::Node {
        GDCLASS(Core, godot::Node)
        protected:
            inline static Core* singleton = nullptr;
            inline static godot::WorldEnvironment* environment = nullptr;
            static void _bind_methods() {
                godot::ClassDB::bind_method(godot::D_METHOD("add_child_node", "node"),                        &Core::add_child_node);
                godot::ClassDB::bind_method(godot::D_METHOD("setup_model_spawner"),                           &Core::setup_model_spawner);
                godot::ClassDB::bind_method(godot::D_METHOD("spawn_model", "name", "authority_peer"),         &Core::spawn_model);
            };
        public:
            // Instantiators //
            Core() = default;
            ~Core() override = default;
            void _ready() override;
            void _exit_tree() override;
            void _process(double delta) override;
            #if defined(Vital_SDK_Client)
            void _unhandled_input(godot::Ref<godot::InputEvent> event);
            #endif


            // Getters //
            static Core* get_singleton();
            static godot::Node* get_root();
            #if defined(Vital_SDK_Client)
            static godot::DisplayServer* get_display_server();
            static godot::RenderingServer* get_rendering_server();
            static godot::Ref<godot::Environment> get_environment();
            static void free_environment();
            #endif

            // Helpers //
            void add_child_node(godot::Node* node);
            void setup_model_spawner();
            void spawn_model(const godot::String& name, int authority_peer);

            // Teardown //
            static void teardown();
    };
}