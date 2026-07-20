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
#include <Vital.sandbox/Vital/sandbox.h>


//////////////////////////
// Vital: Engine: Core //
//////////////////////////

namespace Vital::Engine {
    class Core : public godot::Node {
        GDCLASS(Core, godot::Node)
        protected:
            inline static Core* singleton = nullptr;
            #if defined(VSDK_Client)
            inline static godot::WorldEnvironment* environment = nullptr;
            #endif
            std::mutex mutex;
            std::thread kit_thread;
            std::atomic<bool> kit_ready { false };
            std::atomic<bool> kit_abort { false };
            std::vector<std::function<void()>> work_queue;
            #if defined(VSDK_Client)
            Tool::HTTP::Server http_server;
            #endif
        private:
            // Instantiators //
            Core() = default;
            ~Core() override = default;

            static void _bind_methods() {
                godot::ClassDB::bind_method(godot::D_METHOD("drain"), &Core::drain);
            };
        public:
            // Hooks //
            void _ready() override;
            void _exit_tree() override;
            void _process(double delta) override;
            #if defined(VSDK_Client)
            void _unhandled_input(godot::Ref<godot::InputEvent> event);
            #endif


            // Singleton //
            static Core* get_singleton();
            static void free_singleton();


            // Managers //
            bool is_ready();
            void execute(std::function<void()> exec);
            void enqueue(std::function<void()> exec);
            void drain();
            void teardown();
            void shutdown();


            // Misc //
            static godot::SceneTree* get_scene_tree();
            static godot::Window* get_scene_root();
            #if defined(VSDK_Client)
            static godot::DisplayServer* get_display_server();
            static godot::RenderingServer* get_rendering_server();
            static godot::Ref<godot::Environment> get_environment();
            static godot::Ref<godot::Sky> get_sky();
            static void free_environment();
            static void reset_environment();
            godot::Vector2 get_resolution();
            std::string take_screenshot(const std::string& path = "", const std::string& format = "png");
            std::string get_http_url(const std::string& path = "") const;
            #endif
    };
}