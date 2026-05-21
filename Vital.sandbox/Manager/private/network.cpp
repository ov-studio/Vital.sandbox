/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: network.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/network.h>
#include <Vital.sandbox/Engine/public/console.h>


//////////////////////////////
// Vital: Manager: Network //
//////////////////////////////

namespace Vital::Manager {
    // TODO: Improve

    //--------------------//
    //   Network: Init    //
    //--------------------//

    godot::SceneTree* Network::get_scene_tree() {
        return godot::Object::cast_to<godot::SceneTree>(
            godot::Engine::get_singleton() -> get_main_loop()
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
        node = memnew(Engine::Network);
        node -> set_name("VitalNetwork");
        Engine::Core::get_scene_root() -> add_child(node);
        node -> setup_rpc();

        #if defined(Vital_SDK_Client)
        node -> on_connected_to_server = [this]() { _on_connected_to_server(); };
        node -> on_connection_failed   = [this]() { _on_connection_failed();   };
        node -> on_server_disconnected = [this]() { _on_server_disconnected(); };
        #else
        node -> on_peer_connected    = [this](int id) { _on_peer_connected(id);    };
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
        if (!node) { Tool::print("sbox", "wire_client_signals: no node"); return; }
        auto mp = get_scene_tree() -> get_multiplayer();
        if (!mp.is_valid()) { Tool::print("sbox", "wire_client_signals: mp invalid"); return; }
        mp -> connect("connected_to_server", godot::Callable(node, "_on_connected_to_server"));
        mp -> connect("connection_failed",   godot::Callable(node, "_on_connection_failed"));
        mp -> connect("server_disconnected", godot::Callable(node, "_on_server_disconnected"));
    }
    #else
    void Network::wire_server_signals() {
        if (!node) return;
        auto mp = get_scene_tree() -> get_multiplayer();
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
            if (mp -> is_connected(signal, cb)) mp -> disconnect(signal, cb);
        };
        #if defined(Vital_SDK_Client)
        try_disconnect("connected_to_server", "_on_connected_to_server");
        try_disconnect("connection_failed",   "_on_connection_failed");
        try_disconnect("server_disconnected", "_on_server_disconnected");
        #else
        try_disconnect("peer_connected",    "_on_peer_connected");
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

    int Network::get_peer_id() const {
        auto tree = get_scene_tree();
        if (!tree) return 0;
        auto mp = tree -> get_multiplayer();
        return mp.is_valid() ? mp -> get_unique_id() : 0;
    }


    //--------------------//
    //  Packet Received   //
    //--------------------//

    void Network::_on_packet_received(godot::Dictionary data) {
        auto tree = get_scene_tree();
        if (!tree) return;
        auto mp = tree -> get_multiplayer();
        int32_t sender = mp.is_valid() ? mp -> get_remote_sender_id() : 0;
        godot::Dictionary obj = data.has("object") ? (godot::Dictionary)data["object"] : godot::Dictionary();
        obj["sender_id"] = (int64_t)sender;
        data["object"] = obj;
        Tool::Event::emit("vital.network:packet", Tool::Stack::from_dict(data));
    }

    #if defined(Vital_SDK_Client)
    bool Network::connect_to_server(const std::string& ip, int port, bool enable_reconnect) {
        if (is_active() || is_connecting()) {
            Tool::print("sbox", "Network: already connected/connecting");
            return false;
        }
        create_node();
        peer.instantiate();
        if (peer -> create_client(godot::String(ip.c_str()), port) != godot::OK) {
            Tool::print("sbox", "Network: failed to connect to ", ip.c_str(), ":", port);
            peer.unref();
            return false;
        }
        auto tree = get_scene_tree();
        if (!tree) { peer.unref(); return false; }
        tree -> get_multiplayer() -> set_multiplayer_peer(peer);
        wire_client_signals();
        auto_reconnect     = enable_reconnect;
        reconnect_ip       = ip;
        reconnect_port     = port;
        reconnect_attempts = 0;
        reconnect_timer    = 0.0f;
        pending_handshake  = false;
        Tool::print("sbox", "Network: connecting to ", ip.c_str(), ":", port);
        Tool::Event::emit("vital.network:connect", {});
        return true;
    }

    bool Network::disconnect_from_server() {
        if (!peer.is_valid()) return false;
        auto_reconnect    = false;
        pending_handshake = false;
        unwire_signals();
        peer -> close();
        peer.unref();
        auto tree = get_scene_tree();
        if (tree) tree -> get_multiplayer() -> set_multiplayer_peer(nullptr);
        Tool::print("sbox", "Network: disconnected");
        Tool::Event::emit("vital.network:disconnect", {});
        return true;
    }

    void Network::_on_connected_to_server() {
        reconnect_attempts = 0;
        pending_handshake  = true;
        Tool::print("sbox", "Network: connected (handshake deferred)");
        Tool::Event::emit("vital.network:connect:success", {});
    }

    void Network::_on_connection_failed() {
        Tool::print("sbox", "Network: connection failed");
        pending_handshake = false;
        unwire_signals();
        if (peer.is_valid()) peer.unref();
        Tool::Event::emit("vital.network:connect:failed", {});
        if (auto_reconnect) _schedule_reconnect();
    }

    void Network::_on_server_disconnected() {
        Tool::print("sbox", "Network: server dropped connection");
        pending_handshake = false;
        unwire_signals();
        if (peer.is_valid()) peer.unref();
        Tool::Event::emit("vital.network:server:disconnect", {});
        if (auto_reconnect) _schedule_reconnect();
    }

    void Network::set_reconnect_config(int max_attempts, float delay_seconds) {
        reconnect_max   = max_attempts;
        reconnect_delay = delay_seconds;
    }

    void Network::_schedule_reconnect() {
        if (reconnect_attempts >= reconnect_max) {
            Tool::print("sbox", "Network: max reconnect attempts reached");
            auto_reconnect = false;
            Tool::Event::emit("vital.network:reconnect:failed", {});
            return;
        }
        reconnect_attempts++;
        reconnect_timer = reconnect_delay;
        Tool::print("sbox", "Network: retry in ", reconnect_delay, "s  attempt ", reconnect_attempts, "/", reconnect_max);
        Tool::Event::emit("vital.network:reconnect", {});
    }

    std::string Network::get_server_ip() const { return reconnect_ip; }

    #else
    bool Network::host(Config::Server& config) {
        if (is_active()) {
            Tool::print("sbox", "Network: already hosting");
            return false;
        }
        server_config = &config;
        create_node();
        peer.instantiate();
        godot::Error err = peer -> create_server(config.get_network_port(), config.get_max_clients());
        if (err != godot::OK) {
            Tool::print("sbox", "Network: failed to host on port ", config.get_network_port(), " (err=", (int)err, ")");
            peer.unref();
            server_config = nullptr;
            return false;
        }
        auto tree = get_scene_tree();
        if (!tree) { peer.unref(); server_config = nullptr; return false; }
        tree -> get_multiplayer() -> set_multiplayer_peer(peer);
        wire_server_signals();
        try {
            server_ip = Tool::HTTP::get("https://api.ipify.org", {}, 10);
            if (!server_ip.empty() && std::isspace((unsigned char)server_ip.back())) server_ip.pop_back();
        }
        catch (...) { }
        Vital::Tool::print("sbox", fmt::format(
            "Network: Server is live!\n"
            "> IP — `{}`\n"
            "> Port — `{}`",
            get_server_ip(),
            get_server_config().get_network_port()
        ));
        Tool::Event::emit("vital.network:host", {});
        return true;
    }

    bool Network::close() {
        if (!peer.is_valid()) return false;
        unwire_signals();
        connected_peers.clear();
        peer -> close();
        peer.unref();
        auto tree = get_scene_tree();
        if (tree) tree -> get_multiplayer() -> set_multiplayer_peer(nullptr);
        Tool::print("sbox", "Network: server closed");
        Tool::Event::emit("vital.network:close", {});
        return true;
    }

    void Network::_on_peer_connected(int id) {
        connected_peers.insert(id);
        Tool::print("sbox", "Network: peer joined -> ", id, "  total: ", (int)connected_peers.size());
        Tool::Stack args;
        args.array.push_back(Tool::StackValue((int32_t)id));
        Tool::Event::emit("vital.network:peer:join", args);
    }

    void Network::_on_peer_disconnected(int id) {
        connected_peers.erase(id);
        Tool::print("sbox", "Network: peer left -> ", id, "  remaining: ", (int)connected_peers.size());
        Tool::Stack args;
        args.array.push_back(Tool::StackValue((int32_t)id));
        Tool::Event::emit("vital.network:peer:leave", args);
    }

    const std::unordered_set<int>& Network::get_connected_peers() const { 
        return connected_peers; 
    }

    int Network::get_peer_count() const { 
        return static_cast<int>(connected_peers.size());
    }

    const Config::Server& Network::get_server_config() const { 
        return *server_config;
    }

    std::string Network::get_server_ip() const { 
        return server_ip;
    }
    #endif


    //--------------------//
    //   Send / Receive   //
    //--------------------//

    bool Network::send(const Tool::Stack& stack, int peerID) {
        if (!node || !peer.is_valid()) return false;
        #if defined(Vital_SDK_Client)
        if (!is_active()) return false;
        #endif
        if (peerID == 0) node -> rpc("_receive", stack.to_dict());
        else node -> rpc_id(peerID, "_receive", stack.to_dict());
        return true;
    }

    bool Network::broadcast(const Tool::Stack& stack)      { return send(stack, 0); }
    bool Network::send_to_server(const Tool::Stack& stack) { return send(stack, 1); }
    void Network::emit(Tool::Stack& arguments, int peerID) { get_singleton() -> send(arguments, peerID); }


    //--------//
    //  Poll  //
    //--------//

    void Network::poll(double delta) {
        #if defined(Vital_SDK_Client)
        if (auto_reconnect && !is_active() && !is_connecting()) {
            if (reconnect_timer > 0.0f) {
                reconnect_timer -= static_cast<float>(delta);
                if (reconnect_timer <= 0.0f)
                    connect_to_server(reconnect_ip, reconnect_port, true);
            }
            return;
        }
        if (pending_handshake && is_active()) {
            pending_handshake = false;
            Tool::print("sbox", "Network: sending handshake, peer_id=", get_peer_id());
            Tool::Stack msg;
            msg.array.push_back(Tool::StackValue(std::string("ping")));
            msg.object["event"] = Tool::StackValue(std::string("system"));
            send_to_server(msg);
        }
        #endif
    }
}