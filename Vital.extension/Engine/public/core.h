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
    // TODO: Improve
    class Core : public godot::Node {
        GDCLASS(Core, godot::Node)
        protected:
            inline static Core* singleton  = nullptr;
            inline static godot::WorldEnvironment* environment = nullptr;
            std::thread kit_thread;
            std::atomic<bool> kit_ready { false };
            std::atomic<bool> kit_abort { false };
            std::mutex deferred_mutex;
            std::vector<std::function<void()>> deferred_queue;

            static void _bind_methods() {
                godot::ClassDB::bind_method(godot::D_METHOD("free_singleton"), &Core::free_singleton);
                godot::ClassDB::bind_method(godot::D_METHOD("setup_model_spawner"), &Core::setup_model_spawner);
                godot::ClassDB::bind_method(godot::D_METHOD("spawn_model", "name", "authority_peer"), &Core::spawn_model);
                godot::ClassDB::bind_method(godot::D_METHOD("on_asset_downloaded", "path"), &Core::on_asset_downloaded);
                godot::ClassDB::bind_method(godot::D_METHOD("on_asset_download_failed", "path"), &Core::on_asset_download_failed);
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
            static void teardown_singleton();


            // Queue //
            void push_deferred(std::function<void()> fn);
            template<typename Fn, typename... Args>
            void push_deferred_call(Fn&& fn, Args&&... args) {
                push_deferred(std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
            }
            void flush_deferred_queue();


            // Getters //
            static godot::SceneTree* get_scene_tree();
            static godot::Window* get_scene_root();
            #if defined(Vital_SDK_Client)
            static godot::DisplayServer* get_display_server();
            static godot::RenderingServer* get_rendering_server();
            static godot::Ref<godot::Environment> get_environment();
            #endif


            // Freers //
            #if defined(Vital_SDK_Client)
            static void free_environment();
            #endif


            // APIs //
            void setup_model_spawner();
            void spawn_model(const godot::String& name, int authority_peer);
            void on_asset_downloaded(const godot::String& path);
            void on_asset_download_failed(const godot::String& path);
    };
}