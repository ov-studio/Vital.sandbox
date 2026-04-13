/*----------------------------------------------------------------
     Resource: Vital.sandbox
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
#include <Vital.sandbox/Manager/public/engine.h>

// TODO: Improve / Remove later

/////////////
// Events //
/////////////

// Global config instance - loaded once at startup
#if !defined(Vital_SDK_Client)
#include <Engine/public/srvconfig.h>
static Vital::Engine::SrvConfig g_server_config;
#endif

// Load server configuration from config.yaml
#if !defined(Vital_SDK_Client)
void load_server_config() {
    bool loaded = g_server_config.load();
    if (!loaded) {
        Vital::Tool::print("sbox", "SrvConfig: No config.yaml found, using default values");
    } else {
        Vital::Tool::print("sbox", "SrvConfig: Server name: '", g_server_config.get_server_name(), "'");
        Vital::Tool::print("sbox", "SrvConfig: Network port: ", g_server_config.get_network_port());
        Vital::Tool::print("sbox", "SrvConfig: HTTP port: ", g_server_config.get_http_port());
        Vital::Tool::print("sbox", "SrvConfig: Max clients: ", g_server_config.get_max_clients());
    }
    
    // Set server info on Asset for /info endpoint
    Vital::Manager::ServerInfo info;
    if (g_server_config.is_loaded()) {
        info.name = g_server_config.get_server_name();
        info.version = g_server_config.get_server_version();
        info.description = g_server_config.get_server_description();
        info.max_clients = g_server_config.get_max_clients();
        info.discord = g_server_config.get_discord_invite();
        info.website = g_server_config.get_website();
    }
    Vital::Manager::Asset::get_singleton() -> set_server_info(info);
}
#endif

// TODO: Improve
void shutdown() {
    #if !defined(Vital_SDK_Client)
    Vital::Engine::Network::get_singleton() -> close();
    #endif
    #if defined(Vital_SDK_Client)
    Vital::Engine::Network::get_singleton() -> disconnect_from_server();
    #endif
    Vital::Engine::Network::free_singleton();
    Vital::Manager::Asset::free_singleton();
}

void setup() {
    auto* net = Vital::Engine::Network::get_singleton();

    #if defined(Vital_SDK_Client)
    Vital::Tool::Event::bind("vital.network:connect", [](Vital::Tool::Stack&) {
        Vital::Tool::print("sbox", "Connecting...");
    });

    Vital::Tool::Event::bind("vital.network:packet", [](Vital::Tool::Stack& args) {
        if (!args.object.count("event")) return;
        const std::string event = args.object.at("event").as<std::string>();
        if (event == "asset:manifest") {
            Vital::Manager::Asset::get_singleton() -> receive_manifest(args);
            return;
        }
    });

    Vital::Tool::Event::bind("vital.network:connect:failed", [](Vital::Tool::Stack&) {
        Vital::Tool::print("sbox", "Failed to connect");
    });

    Vital::Tool::Event::bind("vital.network:reconnect", [](Vital::Tool::Stack&) {
        Vital::Tool::print("sbox", "Retrying...");
    });

    Vital::Tool::Event::bind("vital.network:reconnect:failed", [](Vital::Tool::Stack&) {
        Vital::Tool::print("sbox", "Gave up reconnecting");
    });

    Vital::Tool::Event::bind("vital.network:disconnect", [](Vital::Tool::Stack&) {
        Vital::Tool::print("sbox", "Disconnected cleanly");
    });

    net->set_reconnect_config(5, 3.0f);
    #else
    Vital::Tool::Event::bind("vital.network:host", [](Vital::Tool::Stack&) {
        Vital::Tool::print("sbox", "Server is live");
    });

    Vital::Tool::Event::bind("vital.network:peer:join", [](Vital::Tool::Stack& args) {
        int32_t id = args.array[0].as<int32_t>();
        Vital::Tool::print("sbox", "Player joined: ", id);
    });

    Vital::Tool::Event::bind("vital.network:peer:leave", [](Vital::Tool::Stack& args) {
        int32_t id = args.array[0].as<int32_t>();
        Vital::Tool::print("sbox", "Player left: ", id);
    });

    Vital::Tool::Event::bind("vital.network:close", [](Vital::Tool::Stack&) {
        Vital::Tool::print("sbox", "Server closed");
    });
    #endif
}

void initialize_vital_events() {
    // Core //
    Vital::Tool::Event::bind("vital.core:ready", [](Vital::Tool::Stack arguments) {
        #if defined(Vital_SDK_Client)
        Vital::Engine::Canvas::get_singleton();
        Vital::Manager::Discord::get_singleton();
        #endif
        Vital::Engine::Console::get_singleton();
        Vital::Manager::Sandbox::get_singleton() -> ready();
        Vital::Manager::Resource::get_singleton() -> init();
        setup();
    });

    Vital::Tool::Event::bind("vital.core:free", [](Vital::Tool::Stack arguments) {
        #if defined(Vital_SDK_Client)
        Vital::Engine::Canvas::free_singleton();
        Vital::Manager::Discord::free_singleton();
        #endif
        Vital::Engine::Console::free_singleton();
        Vital::Manager::Sandbox::free_singleton();
        Vital::Manager::Resource::free_singleton();
        shutdown();
    });


    // Sandbox //
    Vital::Tool::Event::bind("vital.sandbox:ready", [](Vital::Tool::Stack arguments) {
        Vital::Engine::Model::setup_spawner(); // TODO: LATER THIS ME PART OF SOME MODEL:init() imo
    });


    // Network //
    #if !defined(Vital_SDK_Client)
    Vital::Tool::Event::bind("vital.network:peer:leave", [](Vital::Tool::Stack& args) {
        int32_t id = args.array[0].as<int32_t>();
        Vital::Engine::Model::clear_synced();
    });
    #endif

    #if defined(Vital_SDK_Client)
    Vital::Tool::Event::bind("vital.network:connect:success", [](Vital::Tool::Stack&) {
        auto* net = Vital::Engine::Network::get_singleton();
        Vital::Tool::print("sbox", "Connected! My ID: ", net->get_peer_id());
        Vital::Engine::Model::on_connected();
        Vital::Manager::Asset::get_singleton() -> clear();
        Vital::Manager::Asset::get_singleton() -> set_server_http_ip(net->get_server_ip());
    });

    Vital::Tool::Event::bind("vital.network:server:disconnect", [](Vital::Tool::Stack&) {
        Vital::Tool::print("sbox", "Lost connection to server");
        Vital::Engine::Model::cleanup_spawned();
        Vital::Manager::Asset::get_singleton() -> clear();
    });
    #endif


    // Process //
    Vital::Tool::Event::bind("vital.sandbox:process", [](Vital::Tool::Stack arguments) {
        static bool network_initialized = false;
        if (!network_initialized) {
            network_initialized = true;
            auto* net = Vital::Engine::Network::get_singleton();
            #if !defined(Vital_SDK_Client)
            // Use config values for server settings
            load_server_config();
            int server_port = g_server_config.is_loaded() ? g_server_config.get_network_port() : 7777;
            int max_clients = g_server_config.is_loaded() ? g_server_config.get_max_clients() : 32;
            
            Vital::Tool::print("sbox", "Starting server on port ", server_port, " (max ", max_clients, " clients)");
            net->host(server_port, max_clients);
            
            // Configure HTTP server port before starting
            int http_port = g_server_config.is_loaded() ? g_server_config.get_http_port() : 7778;
            Vital::Manager::Asset::get_singleton() -> set_http_port(http_port);
            
            // Start HTTP asset server
            Vital::Tool::print("sbox", "Starting HTTP server on port ", http_port);
            Vital::Manager::Asset::get_singleton() -> start_http_server();
            #endif
            #if defined(Vital_SDK_Client)
            net->connect_to_server("127.0.0.1", 7777, true);
            #endif
        }

        #if defined(Vital_SDK_Client)
        Vital::Manager::Discord::get_singleton() -> process();
        #endif
        Vital::Engine::Network::get_singleton() -> poll(arguments.array[0].as<double>());
    });
}
