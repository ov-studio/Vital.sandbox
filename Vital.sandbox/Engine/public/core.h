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
                godot::ClassDB::bind_method(godot::D_METHOD("shutdown"), &Core::shutdown);
                #if defined(VSDK_Benchmark)
                ADD_SIGNAL(godot::MethodInfo("native_event", godot::PropertyInfo(godot::Variant::STRING, "name"), godot::PropertyInfo(godot::Variant::DICTIONARY, "payload")));
                #endif
            };
        public:
            // Hooks //
            void _ready() override;
            void _exit_tree() override;
            void _process(double delta) override;
            void _physics_process(double delta) override;
            #if defined(VSDK_Client)
            void _unhandled_input(godot::Ref<godot::InputEvent> event);
            #endif


            // Singleton //
            static Core* get_singleton();
            static void free_singleton();


            // Managers //
            bool is_ready();
            #if defined(VSDK_Client)
            bool is_sandbox_ui_ready();
            bool is_sandbox_ui_visible();
            #endif
            void execute(std::function<void()> exec);
            void enqueue(std::function<void()> exec);
            void execute_when_ready(godot::Node3D* node, godot::Node* target, std::function<void(godot::Node3D*, godot::Node*)> exec);
            #if defined(VSDK_Benchmark)
            void emit_native_event(const std::string& name, const Tool::Stack& payload);
            #endif
            void drain();
            void teardown();
            void shutdown();

            // TODO: Improve
            // Full reset of everything a connected session accumulated, without
            // tearing down Core/the engine process itself — for a client that's
            // leaving one server to go back to a menu / connect to a different one
            // (unlike shutdown(), which is for quitting the app). Stops every
            // running resource (unloading its Lua environment), destroys every
            // networked entity this client currently knows about, clears the model
            // asset cache, and finally frees the Sandbox singleton outright so the
            // Lua VM itself is gone — the next get_singleton() call anywhere builds
            // a completely fresh one with nothing left over from this session.
            // Hooked to both "network:server:disconnect" and "network:disconnect"
            // in event.cpp, so it runs the same way whether the server dropped us
            // or we chose to leave.
            void session_end();


            // Misc //
            static godot::SceneTree* get_scene_tree();
            static godot::Window* get_scene_root();
            #if defined(VSDK_Client)
            static godot::DisplayServer* get_display_server();
            static godot::RenderingServer* get_rendering_server();
            static godot::AudioServer* get_audio_server();
            static godot::Ref<godot::Environment> get_environment();
            static godot::Ref<godot::Sky> get_sky();
            static void free_environment();
            static void reset_environment();
            godot::Vector2 get_resolution();
            std::string get_http_url(const std::string& path = "") const;
            void capture_screenshot(const std::string& base, const std::string& path);
            #endif
    };
}
