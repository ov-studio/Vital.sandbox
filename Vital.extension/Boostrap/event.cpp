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

void shutdown() {
    auto* net = Vital::Engine::Network::get_singleton();
    if (Vital::Engine::Network::is_server()) net->close();
    else net->disconnect_from_server();
    Vital::Engine::Network::free_singleton();
}

void setup() {
    auto* net = Vital::Engine::Network::get_singleton();

    if (Vital::Engine::Network::is_server()) {
        Vital::Tool::Event::bind("network:hosted", [](Vital::Tool::Stack&) {
            Vital::print("sbox", "Server is live");
        });
        Vital::Tool::Event::bind("network:peer_joined", [](Vital::Tool::Stack& args) {
            int32_t id = args.array[0].as<int32_t>();
            Vital::print("sbox", "Player joined: ", id);
        });
        Vital::Tool::Event::bind("network:peer_left", [](Vital::Tool::Stack& args) {
            int32_t id = args.array[0].as<int32_t>();
            Vital::print("sbox", "Player left: ", id);
        });
        Vital::Tool::Event::bind("network:packet", [net](Vital::Tool::Stack& args) {
            int32_t sender = args.object.at("sender_id").as<int32_t>();
            std::string msg = args.array[0].as<std::string>();
            Vital::print("sbox", "Msg from ", sender, ": ", msg.c_str());
            Vital::Tool::Stack reply;
            reply.array.push_back(Vital::Tool::StackValue(std::string("pong")));
            net->send(reply, sender);
        });
        Vital::Tool::Event::bind("network:closed", [](Vital::Tool::Stack&) {
            Vital::print("sbox", "Server closed");
        });
    } else {
        Vital::Tool::Event::bind("network:connecting", [](Vital::Tool::Stack&) {
            Vital::print("sbox", "Connecting...");
        });
        Vital::Tool::Event::bind("network:connected", [net](Vital::Tool::Stack&) {
            Vital::print("sbox", "Connected! My ID: ", net->get_peer_id());
            Vital::Tool::Stack msg;
            msg.array.push_back(Vital::Tool::StackValue(std::string("ping")));
            net->send_to_server(msg);
        });
        Vital::Tool::Event::bind("network:packet", [](Vital::Tool::Stack& args) {
            std::string msg = args.array[0].as<std::string>();
            Vital::print("sbox", "Server replied: ", msg.c_str());
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
        Vital::Tool::Event::bind("network:server_disconnected", [](Vital::Tool::Stack&) {
            Vital::print("sbox", "Lost connection to server");
        });
        Vital::Tool::Event::bind("network:disconnected", [](Vital::Tool::Stack&) {
            Vital::print("sbox", "Disconnected cleanly");
        });
        net->set_reconnect_config(5, 3.0f);
    }
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

    });

    Vital::Tool::Event::bind("vital.sandbox:process", [](Vital::Tool::Stack arguments) -> void {
        #if defined(Vital_SDK_Client)
        Vital::System::Discord::get_singleton() -> process();
        #endif

        static bool network_started = false;
        if (!network_started) {
            network_started = true;
            auto* net = Vital::Engine::Network::get_singleton();
            if (Vital::Engine::Network::is_server()) net->host(7777, 32);
            else net->connect_to_server("127.0.0.1", 7777, true);
        }

        Vital::Engine::Network::get_singleton()->poll(arguments.array[0].as<double>());
        

    });
}