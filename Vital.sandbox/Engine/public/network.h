/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: network.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>


/////////////////////////////
// Vital: Engine: Network //
/////////////////////////////

namespace Vital::Engine {
    class Network : public godot::Node {
        GDCLASS(Network, godot::Node)
        public:
            static void _bind_methods();
            void _receive(godot::Dictionary data);
            void setup_rpc();

            #if defined(Vital_SDK_Client)
            std::function<void()> on_connected_to_server;
            std::function<void()> on_connection_failed;
            std::function<void()> on_server_disconnected;
            void _on_connected_to_server();
            void _on_connection_failed();
            void _on_server_disconnected();
            #else
            std::function<void(int)> on_peer_connected;
            std::function<void(int)> on_peer_disconnected;
            void _on_peer_connected(int id);
            void _on_peer_disconnected(int id);
            #endif
    };
}