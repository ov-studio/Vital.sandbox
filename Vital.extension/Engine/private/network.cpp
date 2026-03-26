/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: network.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/core.h>
#include <Vital.extension/Engine/public/network.h>
#include <Vital.extension/Engine/public/console.h>


/////////////////////////////
// Vital: Engine: Network //
/////////////////////////////

namespace Vital::Engine {
    // TODO: Improve

    //--------------------//
    //    NetworkNode     //
    //--------------------//

    void NetworkNode::_bind_methods() {
        // Bind _receive so Godot knows it exists as a callable method.
        // RPC config is set in setup_rpc() after the node is in the tree.
        godot::ClassDB::bind_method(godot::D_METHOD("_receive", "data"), &NetworkNode::_receive);
        godot::ClassDB::bind_method(godot::D_METHOD("setup_rpc"), &NetworkNode::setup_rpc);
        #if defined(Vital_SDK_Client)
        godot::ClassDB::bind_method(godot::D_METHOD("_on_connected_to_server"), &NetworkNode::_on_connected_to_server);
        godot::ClassDB::bind_method(godot::D_METHOD("_on_connection_failed"), &NetworkNode::_on_connection_failed);
        godot::ClassDB::bind_method(godot::D_METHOD("_on_server_disconnected"), &NetworkNode::_on_server_disconnected);
        #else
        godot::ClassDB::bind_method(godot::D_METHOD("_on_peer_connected", "id"), &NetworkNode::_on_peer_connected);
        godot::ClassDB::bind_method(godot::D_METHOD("_on_peer_disconnected", "id"), &NetworkNode::_on_peer_disconnected);
        #endif
    }

    // Must be called AFTER the node is added to the SceneTree.
    // set_rpc_config() requires a valid node path which only exists in-tree.
    void NetworkNode::setup_rpc() {
        godot::Dictionary cfg;
        cfg["rpc_mode"]      = (int)godot::MultiplayerAPI::RPC_MODE_ANY_PEER;
        cfg["transfer_mode"] = (int)godot::MultiplayerPeer::TRANSFER_MODE_RELIABLE;
        cfg["call_local"]    = false;
        cfg["channel"]       = 0;
        rpc_config("_receive", cfg);
    }

    // Single RPC entry point — Godot routes all incoming packets here.
    // sender_id is retrieved from MultiplayerAPI::get_remote_sender_id().
    void NetworkNode::_receive(godot::Dictionary data) {
        Network::get_singleton() -> _on_packet_received(data);
    }

    #if defined(Vital_SDK_Client)
    void NetworkNode::_on_connected_to_server() { if (on_connected_to_server) on_connected_to_server(); }
    void NetworkNode::_on_connection_failed() { if (on_connection_failed) on_connection_failed(); }
    void NetworkNode::_on_server_disconnected() { if (on_server_disconnected) on_server_disconnected(); }
    #else
    void NetworkNode::_on_peer_connected(int id) { if (on_peer_connected) on_peer_connected(id); }
    void NetworkNode::_on_peer_disconnected(int id) { if (on_peer_disconnected) on_peer_disconnected(id); }
    #endif


    //--------------------//
    //   Network: Init    //
    //--------------------//

    godot::SceneTree* Network::get_scene_tree() {
        return godot::Object::cast_to<godot::SceneTree>(
            godot::Engine::get_singleton()->get_main_loop()
        );
    }

    Network* Network::get_singleton() {
        if (!singleton) singleton = new Network();
        return singleton;
    }

    void Network::free_singleton() {
        if (!singleton) return;
        #if defined(Vital_SDK_Client)
        singleton -> disconnect_from_server();
        #else
        singleton -> close();
        #endif
        singleton -> unwire_signals();
        singleton -> destroy_node();
        if (singleton -> peer.is_valid()) {
            singleton -> peer -> close();
            singleton -> peer.unref();
        }
        delete singleton;
        singleton = nullptr;
    }


    //--------------------//
    //    Node Helpers    //
    //--------------------//

    void Network::create_node() {
        if (node) return;
        node = memnew(NetworkNode);
        node -> set_name("VitalNetwork");
        Core::get_scene_root() -> add_child(node);
        // setup_rpc() must be called after add_child so the node has a valid path
        node -> setup_rpc();

        #if defined(Vital_SDK_Client)
        node -> on_connected_to_server = [this]() { _on_connected_to_server(); };
        node -> on_connection_failed = [this]() { _on_connection_failed(); };
        node -> on_server_disconnected = [this]() { _on_server_disconnected(); };
        #else
        node -> on_peer_connected = [this](int id) { _on_peer_connected(id); };
        node -> on_peer_disconnected = [this](int id) { _on_peer_disconnected(id); };
        #endif
    }

    void Network::destroy_node() {
        if (!node) return;
        node -> queue_free();
        node = nullptr;
    }

    #if defined(Vital_SDK_Client)
    void Network::wire_client_signals() {
        if (!node) { Vital::print("sbox", "wire_client_signals: no node"); return; }
        auto mp = get_scene_tree()->get_multiplayer();
        if (!mp.is_valid()) { Vital::print("sbox", "wire_client_signals: mp invalid"); return; }
        mp -> connect("connected_to_server", godot::Callable(node, "_on_connected_to_server"));
        mp -> connect("connection_failed",   godot::Callable(node, "_on_connection_failed"));
        mp -> connect("server_disconnected", godot::Callable(node, "_on_server_disconnected"));
    }
    #else
    void Network::wire_server_signals() {
        if (!node) return;
        auto mp = get_scene_tree()->get_multiplayer();
        if (!mp.is_valid()) return;
        mp -> connect("peer_connected",    godot::Callable(node, "_on_peer_connected"));
        mp -> connect("peer_disconnected", godot::Callable(node, "_on_peer_disconnected"));
    }
    #endif

    void Network::unwire_signals() {
        auto tree = get_scene_tree();
        if (!tree || !node) return;
        auto mp = tree -> get_multiplayer();
        if (!mp.is_valid()) return;
        auto try_disconnect = [&](const char* signal, const char* method) {
            godot::Callable cb(node, method);
            if (mp -> is_connected(signal, cb))
                mp -> disconnect(signal, cb);
        };
        #if defined(Vital_SDK_Client)
        try_disconnect("connected_to_server", "_on_connected_to_server");
        try_disconnect("connection_failed", "_on_connection_failed");
        try_disconnect("server_disconnected", "_on_server_disconnected");
        #else
        try_disconnect("peer_connected", "_on_peer_connected");
        try_disconnect("peer_disconnected", "_on_peer_disconnected");
        #endif
    }


    //----------------//
    //     State      //
    //----------------//

    bool Network::is_active() const {
        return peer.is_valid() && peer -> get_connection_status() == godot::MultiplayerPeer::CONNECTION_CONNECTED;
    }

    bool Network::is_connecting() const {
        return peer.is_valid() && peer -> get_connection_status() == godot::MultiplayerPeer::CONNECTION_CONNECTING;
    }

    bool Network::is_server() {
        return get_platform() == "server";
    }

    int Network::get_peer_id() const {
        auto tree = get_scene_tree();
        if (!tree) return 0;
        auto mp = tree -> get_multiplayer();
        return mp.is_valid() ? mp -> get_unique_id() : 0;
    }

    void Network::print_status() const {
        Vital::print("sbox", "=== Network Status ===");
        Vital::print("sbox", "Role   : ", is_server() ? "SERVER" : "CLIENT");
        Vital::print("sbox", "Active : ", is_active());
        Vital::print("sbox", "PeerID : ", get_peer_id());
        #if !defined(Vital_SDK_Client)
        Vital::print("sbox", "Clients: ", (int)connected_peers.size());
        #endif
    }


    //--------------------//
    //  Packet Received   //
    //--------------------//

    void Network::_on_packet_received(godot::Dictionary data) {
        auto tree = get_scene_tree();
        if (!tree) return;
        auto mp = tree -> get_multiplayer();

        // Godot provides sender ID via get_remote_sender_id() — no peer mapping needed
        int32_t sender = mp.is_valid() ? mp -> get_remote_sender_id() : 0;

        // Inject sender_id into the object map before converting to Stack
        godot::Dictionary obj = data.has("object") ? (godot::Dictionary)data["object"] : godot::Dictionary();
        obj["sender_id"] = (int64_t)sender;
        data["object"]   = obj;

        // Convert to Stack and emit into the event system
        Vital::Tool::Event::emit("network:packet", Vital::Tool::Stack::from_dict(data));
    }

    #if defined(Vital_SDK_Client)
    bool Network::connect_to_server(const std::string& ip, int port, bool enable_reconnect) {
        if (is_active() || is_connecting()) {
            Vital::print("sbox", "Network: already connected/connecting");
            return false;
        }
        create_node();
        peer.instantiate();
        if (peer -> create_client(godot::String(ip.c_str()), port) != godot::OK) {
            Vital::print("sbox", "Network: failed to connect to ", ip.c_str(), ":", port);
            peer.unref();
            return false;
        }
        auto tree = get_scene_tree();
        if (!tree) { peer.unref(); return false; }
        tree -> get_multiplayer()->set_multiplayer_peer(peer);
        wire_client_signals();
        auto_reconnect     = enable_reconnect;
        reconnect_ip       = ip;
        reconnect_port     = port;
        reconnect_attempts = 0;
        reconnect_timer    = 0.0f;
        pending_handshake  = false;
        Vital::print("sbox", "Network: connecting to ", ip.c_str(), ":", port);
        Vital::Tool::Event::emit("network:connecting", {});
        return true;
    }

    bool Network::disconnect_from_server() {
        if (!peer.is_valid()) return false;
        auto_reconnect = false;
        pending_handshake = false;
        unwire_signals();
        peer -> close();
        peer.unref();
        auto tree = get_scene_tree();
        if (tree) tree -> get_multiplayer()->set_multiplayer_peer(nullptr);
        Vital::print("sbox", "Network: disconnected");
        Vital::Tool::Event::emit("network:disconnected", {});
        return true;
    }

    void Network::_on_connected_to_server() {
        reconnect_attempts = 0;
        pending_handshake = true;
        Vital::print("sbox", "Network: connected (handshake deferred)");
        Vital::Tool::Event::emit("network:connected", {});
    }

    void Network::_on_connection_failed() {
        Vital::print("sbox", "Network: connection failed");
        pending_handshake = false;
        unwire_signals();
        if (peer.is_valid()) peer.unref();
        Vital::Tool::Event::emit("network:connection_failed", {});
        if (auto_reconnect) _schedule_reconnect();
    }

    void Network::_on_server_disconnected() {
        Vital::print("sbox", "Network: server dropped connection");
        pending_handshake = false;
        unwire_signals();
        if (peer.is_valid()) peer.unref();
        Vital::Tool::Event::emit("network:server_disconnected", {});
        if (auto_reconnect) _schedule_reconnect();
    }

    void Network::set_reconnect_config(int max_attempts, float delay_seconds) {
        reconnect_max = max_attempts;
        reconnect_delay = delay_seconds;
    }

    void Network::_schedule_reconnect() {
        if (reconnect_attempts >= reconnect_max) {
            Vital::print("sbox", "Network: max reconnect attempts reached");
            auto_reconnect = false;
            Vital::Tool::Event::emit("network:reconnect_failed", {});
            return;
        }
        reconnect_attempts++;
        reconnect_timer = reconnect_delay;
        Vital::print("sbox", "Network: retry in ", reconnect_delay, "s  attempt ", reconnect_attempts, "/", reconnect_max);
        Vital::Tool::Event::emit("network:reconnecting", {});
    }

    #else
    bool Network::host(int port, int max_clients) {
        if (!is_server()) {
            Vital::print("sbox", "Network: host() called on non-server");
            return false;
        }
        if (is_active()) {
            Vital::print("sbox", "Network: already hosting");
            return false;
        }
        create_node();
        peer.instantiate();
        godot::Error err = peer -> create_server(port, max_clients);
        if (err != godot::OK) {
            Vital::print("sbox", "Network: failed to host on port ", port, " (err=", (int)err, ")");
            peer.unref();
            return false;
        }
        auto tree = get_scene_tree();
        if (!tree) { peer.unref(); return false; }
        tree -> get_multiplayer()->set_multiplayer_peer(peer);
        wire_server_signals();
        Vital::print("sbox", "Network: hosting on port ", port);
        Vital::Tool::Event::emit("network:hosted", {});
        return true;
    }

    bool Network::close() {
        if (!peer.is_valid()) return false;
        unwire_signals();
        connected_peers.clear();
        peer -> close();
        peer.unref();
        auto tree = get_scene_tree();
        if (tree) tree -> get_multiplayer()->set_multiplayer_peer(nullptr);
        Vital::print("sbox", "Network: server closed");
        Vital::Tool::Event::emit("network:closed", {});
        return true;
    }

    void Network::_on_peer_connected(int id) {
        connected_peers.insert(id);
        Vital::print("sbox", "Network: peer joined -> ", id, "  total: ", (int)connected_peers.size());
        Vital::Tool::Stack args;
        args.array.push_back(Vital::Tool::StackValue((int32_t)id));
        Vital::Tool::Event::emit("network:peer_joined", args);
    }

    void Network::_on_peer_disconnected(int id) {
        connected_peers.erase(id);
        Vital::print("sbox", "Network: peer left -> ", id, "  remaining: ", (int)connected_peers.size());
        Vital::Tool::Stack args;
        args.array.push_back(Vital::Tool::StackValue((int32_t)id));
        Vital::Tool::Event::emit("network:peer_left", args);
    }

    const std::unordered_set<int>& Network::get_connected_peers() const {
        return connected_peers;
    }

    int Network::get_peer_count() const {
        return static_cast<int>(connected_peers.size());
    }
    #endif



    //--------------------//
    //   Send / Receive   //
    //--------------------//

    bool Network::send(const Vital::Tool::Stack& stack, int peerID) {
        if (!node || !peer.is_valid()) return false;
        #if defined(Vital_SDK_Client)
        if (!is_active()) return false;
        #endif
        if (peerID == 0) node -> rpc("_receive", stack.to_dict());
        else node -> rpc_id(peerID, "_receive", stack.to_dict());
        return true;
    }

    bool Network::broadcast(const Vital::Tool::Stack& stack) {
        return send(stack, 0);
    }

    bool Network::send_to_server(const Vital::Tool::Stack& stack) {
        return send(stack, 1);
    }

    void Network::emit(Vital::Tool::Stack& arguments, int peerID) {
        get_singleton()->send(arguments, peerID);
    }


    //--------//
    //  Poll  //
    //--------//

    void Network::poll(double delta) {
        #if defined(Vital_SDK_Client)
        // Handle reconnect timer
        if (auto_reconnect && !is_active() && !is_connecting()) {
            if (reconnect_timer > 0.0f) {
                reconnect_timer -= static_cast<float>(delta);
                if (reconnect_timer <= 0.0f)
                    connect_to_server(reconnect_ip, reconnect_port, true);
            }
            return;
        }

        // Deferred handshake — wait until first poll() after connection
        // so MultiplayerAPI has fully assigned our peer ID
        if (pending_handshake && is_active()) {
            pending_handshake = false;
            Vital::print("sbox", "Network: sending handshake, peer_id=", get_peer_id());
            Vital::Tool::Stack msg;
            msg.array.push_back(Vital::Tool::StackValue(std::string("ping")));
            msg.object["type"] = Vital::Tool::StackValue(std::string("system"));
            send_to_server(msg);
        }
        #endif
    }
}