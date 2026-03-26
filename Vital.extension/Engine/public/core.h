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
            static void _bind_methods() {
                godot::ClassDB::bind_method(godot::D_METHOD("free_singleton"), &Core::free_singleton);
                godot::ClassDB::bind_method(godot::D_METHOD("add_child_node", "node"), &Core::add_child_node);
                godot::ClassDB::bind_method(godot::D_METHOD("setup_model_spawner"), &Core::setup_model_spawner);
                godot::ClassDB::bind_method(godot::D_METHOD("spawn_model", "name", "authority_peer"), &Core::spawn_model);
                godot::ClassDB::bind_method(godot::D_METHOD("process_asset_chunk", "path"), &Core::process_asset_chunk);
                godot::ClassDB::bind_method(godot::D_METHOD("send_asset_to_peer", "path", "peer_id"), &Core::send_asset_to_peer);
                godot::ClassDB::bind_method(godot::D_METHOD("send_asset_chunk", "path", "hash", "data", "chunk_index", "chunk_total", "peer_id"), &Core::send_asset_chunk);
                godot::ClassDB::bind_method(godot::D_METHOD("on_asset_saved", "path"), &Core::on_asset_saved);
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
            void add_child_node(godot::Node* node);
            void setup_model_spawner();
            void spawn_model(const godot::String& name, int authority_peer);
            void process_asset_chunk(const godot::String& path);
            void send_asset_to_peer(const godot::String& path, int peer_id);
            void send_asset_chunk(const godot::String& path, const godot::String& hash, const godot::String& data, int chunk_index, int chunk_total, int peer_id);
            void on_asset_saved(const godot::String& path);
    };
}