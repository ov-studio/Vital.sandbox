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
        Vital::Engine::AssetManager::get_singleton() -> broadcast_manifest(id);
    });
    Vital::Tool::Event::bind("network:peer_left", [](Vital::Tool::Stack& args) {
        int32_t id = args.array[0].as<int32_t>();
        Vital::print("sbox", "Player left: ", id);
    });
    Vital::Tool::Event::bind("network:packet", [net](Vital::Tool::Stack& args) {
        int32_t sender   = args.object.at("sender_id").as<int32_t>();
        std::string type = args.object.count("type") ? args.object.at("type").as<std::string>() : "";
    
        if (type == "asset:request") {
            std::string path = args.object.at("path").as<std::string>();
            Vital::print("sbox", "AssetManager: peer ", sender, " requested -> ", path.c_str());
            Vital::Engine::Core::get_singleton() -> call_deferred(
                "send_asset_to_peer",
                godot::String(path.c_str()),
                (int)sender
            );
            return;
        }
        else if (type == "asset:cancel") {
            std::string path = args.object.at("path").as<std::string>();
            Vital::print("sbox", "AssetManager: server cancelled -> ", path.c_str());
            Vital::Engine::AssetManager::get_singleton() -> cancel(path);
            return;
        }

        /*
        // Guard array access — only proceed if array has entries
        if (args.array.empty()) return;
    
        std::string body = args.array[0].as<std::string>();
        Vital::print("sbox", "Msg from ", sender, " type=", type.c_str(), ": ", body.c_str());
    
        if (body == "ping") {
            Vital::Tool::Stack reply;
            reply.array.push_back(Vital::Tool::StackValue(std::string("welcome")));
            reply.object["type"]    = Vital::Tool::StackValue(std::string("system"));
            reply.object["peer_id"] = Vital::Tool::StackValue(sender);
            net->send(reply, sender);
        }
        */
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
        if (type == "asset:chunk") {
            Vital::Engine::AssetManager::get_singleton() -> receive_chunk(args);
            return;
        }
        //std::string body = args.array[0].as<std::string>();
        //Vital::print("sbox", "Server says [", type.c_str(), "]: ", body.c_str());
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
    net -> set_reconnect_config(5, 3.0f);
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
        Vital::Engine::ResourceManager::get_singleton()->init();
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
        shutdown();
    });


    // Sandbox //
    Vital::Tool::Event::bind("vital.sandbox:ready", [](Vital::Tool::Stack arguments) -> void {
        Vital::Engine::Core::get_singleton() -> call_deferred("setup_model_spawner");

        #if !defined(Vital_SDK_Client)
        // Register your asset manifest
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
        Vital::print("sbox", "Connected! My ID: ", net -> get_peer_id());
        Vital::Engine::Model::on_connected();
        Vital::Engine::AssetManager::get_singleton() -> clear();
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
            net -> host(7777, 32);
            #endif
            #if defined(Vital_SDK_Client)
            net -> connect_to_server("127.0.0.1", 7777, true);
            #endif
        }

        #if defined(Vital_SDK_Client)
        Vital::System::Discord::get_singleton() -> process();
        #endif
        Vital::Engine::Network::get_singleton() -> poll(arguments.array[0].as<double>());
    });
}