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
    // TODO: Improve
    class Network : public godot::Node {
        GDCLASS(Network, godot::Node)
        private:
            static void _bind_methods() { 
                godot::ClassDB::bind_method(godot::D_METHOD("_receive", "data"), &Network::_receive);
                godot::ClassDB::bind_method(godot::D_METHOD("setup_rpc"), &Network::setup_rpc);
                #if defined(VSDK_Client)
                godot::ClassDB::bind_method(godot::D_METHOD("_on_connected_to_server"), &Network::_on_connected_to_server);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_connection_failed"), &Network::_on_connection_failed);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_server_disconnected"), &Network::_on_server_disconnected);
                #else
                godot::ClassDB::bind_method(godot::D_METHOD("_on_peer_connected", "id"), &Network::_on_peer_connected);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_peer_disconnected", "id"), &Network::_on_peer_disconnected);
                #endif
            }
        public:
            void _receive(godot::Dictionary data);
            void setup_rpc();

            #if defined(VSDK_Client)
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