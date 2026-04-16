/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: core.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Core Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/vital.hpp>


//////////////////////////
// Vital: Engine: Core //
//////////////////////////

namespace Vital::Engine {
    class Core : public godot::Node {
        GDCLASS(Core, godot::Node)
        protected:
            inline static Core* singleton = nullptr;
            inline static godot::WorldEnvironment* environment = nullptr;
            std::thread kit_thread;
            std::atomic<bool> kit_ready { false };
            std::atomic<bool> kit_abort { false };
            std::mutex deferred_mutex;
            std::vector<std::function<void()>> deferred_queue;

            static void _bind_methods() {
                godot::ClassDB::bind_method(godot::D_METHOD("free_singleton"), &Core::free_singleton);
                godot::ClassDB::bind_method(godot::D_METHOD("flush_deferred_queue"), &Core::flush_deferred_queue);
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


            // Utils //
            static Core* get_singleton();
            void free_singleton();


            // Managers //
            bool is_ready();
            void push_deferred(std::function<void()> exec);
            void flush_deferred_queue();
            void teardown();


            // APIs //
            static godot::SceneTree* get_scene_tree();
            static godot::Window* get_scene_root();
            #if defined(Vital_SDK_Client)
            static godot::DisplayServer* get_display_server();
            static godot::RenderingServer* get_rendering_server();
            static godot::Ref<godot::Environment> get_environment();
            static void free_environment();
            #endif
    };
}