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

#if !defined(Vital_SDK_Client)
#include <Engine/public/cfg_server.h>
static Vital::Engine::cfg_server g_server_config;
#endif

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
        Vital::Tool::print("sbox", "Player joined: ", args.array[0].as<int32_t>());
    });
    Vital::Tool::Event::bind("vital.network:peer:leave", [](Vital::Tool::Stack& args) {
        Vital::Tool::print("sbox", "Player left: ", args.array[0].as<int32_t>());
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
        Vital::Manager::Asset::get_singleton() -> init();
        Vital::Manager::Resource::get_singleton();
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
            #if defined(Vital_SDK_Client)
                net->connect_to_server("127.0.0.1", 7777, true);
            #else
                g_server_config.load();
                net->host(g_server_config);
                Vital::Manager::Asset::get_singleton() -> set_http_port(g_server_config.get_http_port());
                Vital::Manager::Asset::get_singleton() -> start_http_server();
            #endif
        }

        #if defined(Vital_SDK_Client)
        Vital::Manager::Discord::get_singleton() -> process();
        #endif
        Vital::Engine::Network::get_singleton() -> poll(arguments.array[0].as<double>());
    });
}