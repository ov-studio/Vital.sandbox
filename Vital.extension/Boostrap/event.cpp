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

// TODO: Improve
void shutdown() {
    #if !defined(Vital_SDK_Client)
    Vital::Engine::Network::get_singleton() -> close();
    #endif
    #if defined(Vital_SDK_Client)
    Vital::Engine::Network::get_singleton() -> disconnect_from_server();
    #endif
    Vital::Engine::Network::free_singleton();
    Vital::Engine::AssetManager::free_singleton();
}

void setup() {
    auto* net = Vital::Engine::Network::get_singleton();

    #if !defined(Vital_SDK_Client)
    Vital::Tool::Event::bind("network:hosted", [](Vital::Tool::Stack&) {
        Vital::print("sbox", "Server is live");
    });

    Vital::Tool::Event::bind("network:peer_joined", [](Vital::Tool::Stack& args) {
        int32_t id = args.array[0].as<int32_t>();
        Vital::print("sbox", "Player joined: ", id);
        // Manifest is broadcast via broadcast_manifest_deferred() when resources start
        // and via the peer_connected event in resource.cpp scan()
        // Direct broadcast here handles clients joining after resources are already running
        Vital::Engine::AssetManager::get_singleton() -> broadcast_manifest(id);
    });

    Vital::Tool::Event::bind("network:peer_left", [](Vital::Tool::Stack& args) {
        int32_t id = args.array[0].as<int32_t>();
        Vital::print("sbox", "Player left: ", id);
    });

    Vital::Tool::Event::bind("network:packet", [net](Vital::Tool::Stack& args) {
        int32_t sender   = args.object.at("sender_id").as<int32_t>();
        std::string type = args.object.count("type") ? args.object.at("type").as<std::string>() : "";
        // All asset delivery is now handled by the HTTP server on port 7778
        // No asset:request or asset:chunk handling needed here
    });

    Vital::Tool::Event::bind("network:closed", [](Vital::Tool::Stack&) {
        Vital::print("sbox", "Server closed");
    });
    #endif

    #if defined(Vital_SDK_Client)
    Vital::Tool::Event::bind("network:connecting", [](Vital::Tool::Stack&) {
        Vital::print("sbox", "Connecting...");
    });

    Vital::Tool::Event::bind("network:packet", [](Vital::Tool::Stack& args) {
        std::string type = args.object.count("type") ? args.object.at("type").as<std::string>() : "unknown";
        if (type == "asset:manifest") {
            Vital::Engine::AssetManager::get_singleton() -> receive_manifest(args);
            return;
        }
        // asset:chunk no longer exists — HTTP handles delivery
    });

    Vital::Tool::Event::bind("network:connection_failed", [](Vital::Tool::Stack&) {
        Vital::print("sbox", "Failed to connect");
    });

    Vital::Tool::Event::bind("network:reconnecting", [](Vital::Tool::Stack&) {
        Vital::print("sbox", "Retrying...");
    });

    Vital::Tool::Event::bind("network:reconnect_failed", [](Vital::Tool::Stack&) {
        Vital::print("sbox", "Gave up reconnecting");
    });

    Vital::Tool::Event::bind("network:disconnected", [](Vital::Tool::Stack&) {
        Vital::print("sbox", "Disconnected cleanly");
    });

    Vital::Tool::Event::bind("asset:ready", [](Vital::Tool::Stack&) {
        Vital::print("sbox", "AssetManager: all assets ready");
    });

    net->set_reconnect_config(5, 3.0f);
    #endif
}

void initialize_vital_events() {
    // Core //
    Vital::Tool::Event::bind("vital.core:ready", [](Vital::Tool::Stack arguments) -> void {
        #if defined(Vital_SDK_Client)
        Vital::Engine::Canvas::get_singleton();
        Vital::System::Discord::get_singleton();
        #endif
        Vital::Engine::Console::get_singleton();
        Vital::Engine::Sandbox::get_singleton() -> ready();
        #if defined(Vital_SDK_Client)
        Vital::Engine::ResourceManager::get_singleton() -> init();
        #endif
        setup();
    });

    Vital::Tool::Event::bind("vital.core:free", [](Vital::Tool::Stack arguments) -> void {
        #if defined(Vital_SDK_Client)
        Vital::Engine::Canvas::free_singleton();
        Vital::System::Discord::free_singleton();
        #endif
        Vital::Engine::Console::free_singleton();
        Vital::Engine::Sandbox::free_singleton();
        Vital::Engine::ResourceManager::free_singleton();
        shutdown();
    });


    // Sandbox //
    Vital::Tool::Event::bind("vital.sandbox:ready", [](Vital::Tool::Stack arguments) -> void {
        Vital::Engine::Core::get_singleton() -> call_deferred("setup_model_spawner");

        #if !defined(Vital_SDK_Client)
        // Register global assets (non-resource assets shared across all clients)
        std::vector<std::string> manifest = {
            "cube.glb",
            "test/testing.glb"
        };
        Vital::Engine::AssetManager::get_singleton() -> register_assets(manifest);
        #endif
    });


    // Network //
    #if !defined(Vital_SDK_Client)
    Vital::Tool::Event::bind("network:peer_left", [](Vital::Tool::Stack& args) -> void {
        int32_t id = args.array[0].as<int32_t>();
        Vital::Engine::Model::clear_synced();
    });
    #endif

    #if defined(Vital_SDK_Client)
    Vital::Tool::Event::bind("network:connected", [](Vital::Tool::Stack&) -> void {
        auto* net = Vital::Engine::Network::get_singleton();
        Vital::print("sbox", "Connected! My ID: ", net->get_peer_id());
        Vital::Engine::Model::on_connected();
        Vital::Engine::AssetManager::get_singleton() -> clear();
        // Store server IP so HTTP downloads know where to connect
        Vital::Engine::AssetManager::get_singleton() -> set_server_http_ip(
            net->get_server_ip()
        );
    });

    Vital::Tool::Event::bind("network:server_disconnected", [](Vital::Tool::Stack&) -> void {
        Vital::print("sbox", "Lost connection to server");
        Vital::Engine::Model::cleanup_spawned();
        Vital::Engine::AssetManager::get_singleton() -> clear();
    });
    #endif


    // Process //
    Vital::Tool::Event::bind("vital.sandbox:process", [](Vital::Tool::Stack arguments) -> void {
        static bool network_initialized = false;
        if (!network_initialized) {
            network_initialized = true;
            auto* net = Vital::Engine::Network::get_singleton();
            #if !defined(Vital_SDK_Client)
            net->host(7777, 32);
            // Start HTTP asset server on port 7778 immediately after hosting
            Vital::Engine::AssetManager::get_singleton() -> start_http_server();
            #endif
            #if defined(Vital_SDK_Client)
            net->connect_to_server("127.0.0.1", 7777, true);
            #endif
        }

        #if defined(Vital_SDK_Client)
        Vital::System::Discord::get_singleton() -> process();
        #endif
        Vital::Engine::Network::get_singleton() -> poll(arguments.array[0].as<double>());
    });
}