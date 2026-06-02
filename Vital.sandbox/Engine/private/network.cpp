/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: network.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/network.h>
#include <Vital.sandbox/Manager/public/network.h>


/////////////////////////////
// Vital: Engine: Network //
/////////////////////////////

namespace Vital::Engine {
    void Network::_bind_methods() {
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

    void Network::setup_rpc() {
        godot::Dictionary cfg;
        cfg["rpc_mode"] = (int)godot::MultiplayerAPI::RPC_MODE_ANY_PEER;
        cfg["transfer_mode"] = (int)godot::MultiplayerPeer::TRANSFER_MODE_RELIABLE;
        cfg["call_local"] = false;
        cfg["channel"] = 0;
        rpc_config("_receive", cfg);
    }

    void Network::_receive(godot::Dictionary data) {
        Manager::Network::get_singleton() -> _on_packet_received(data);
    }

    #if defined(VSDK_Client)
    void Network::_on_connected_to_server() { if (on_connected_to_server) on_connected_to_server(); }
    void Network::_on_connection_failed() { if (on_connection_failed) on_connection_failed(); }
    void Network::_on_server_disconnected() { if (on_server_disconnected) on_server_disconnected(); }
    #else
    void Network::_on_peer_connected(int id) { if (on_peer_connected) on_peer_connected(id); }
    void Network::_on_peer_disconnected(int id) { if (on_peer_disconnected) on_peer_disconnected(id); }
    #endif
}