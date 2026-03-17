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
#include <Vital.extension/Engine/public/network.h>
#include <Vital.extension/Engine/public/console.h>


/////////////////////////////
// Vital: Engine: Network //
/////////////////////////////

namespace Vital::Engine {

    //--------------------//
    //   NetworkBridge    //
    //--------------------//

    void NetworkBridge::_bind_methods() {
        #if !defined(Vital_SDK_Client)
        godot::ClassDB::bind_method(godot::D_METHOD("_on_peer_connected",    "id"), &NetworkBridge::_on_peer_connected);
        godot::ClassDB::bind_method(godot::D_METHOD("_on_peer_disconnected", "id"), &NetworkBridge::_on_peer_disconnected);
        #endif

        #if defined(Vital_SDK_Client)
        godot::ClassDB::bind_method(godot::D_METHOD("_on_connected_to_server"),  &NetworkBridge::_on_connected_to_server);
        godot::ClassDB::bind_method(godot::D_METHOD("_on_connection_failed"),    &NetworkBridge::_on_connection_failed);
        godot::ClassDB::bind_method(godot::D_METHOD("_on_server_disconnected"),  &NetworkBridge::_on_server_disconnected);
        #endif
    }

    #if !defined(Vital_SDK_Client)
    void NetworkBridge::_on_peer_connected(int id)    { if (on_peer_connected)    on_peer_connected(id); }
    void NetworkBridge::_on_peer_disconnected(int id) { if (on_peer_disconnected) on_peer_disconnected(id); }
    #endif

    #if defined(Vital_SDK_Client)
    void NetworkBridge::_on_connected_to_server()     { if (on_connected_to_server) on_connected_to_server(); }
    void NetworkBridge::_on_connection_failed()       { if (on_connection_failed)   on_connection_failed(); }
    void NetworkBridge::_on_server_disconnected()     { if (on_server_disconnected) on_server_disconnected(); }
    #endif


    //--------------------//
    //   Network: Init    //
    //--------------------//

    godot::SceneTree* Network::get_scene_tree() {
        return godot::Object::cast_to<godot::SceneTree>(
            godot::Engine::get_singleton()->get_main_loop()
        );
    }

    Vital::Tool::Stack Network::make_stack(int32_t id) {
        Vital::Tool::Stack s;
        s.array.push_back(Vital::Tool::StackValue(id));
        return s;
    }

    Network* Network::get_singleton() {
        if (!singleton) singleton = new Network();
        return singleton;
    }

    void Network::free_singleton() {
        if (!singleton) return;
        #if !defined(Vital_SDK_Client)
        singleton->close();
        #endif
        #if defined(Vital_SDK_Client)
        singleton->disconnect_from_server();
        #endif
        singleton->unwire_signals();
        singleton->destroy_bridge();
        if (singleton->peer.is_valid()) {
            singleton->peer->close();
            singleton->peer.unref();
        }
        delete singleton;
        singleton = nullptr;
    }


    //--------------------//
    //   Bridge Helpers   //
    //--------------------//

    void Network::create_bridge() {
        if (bridge) return;
        bridge = memnew(NetworkBridge);
        #if !defined(Vital_SDK_Client)
        bridge->on_peer_connected    = [this](int id) { _on_peer_connected(id); };
        bridge->on_peer_disconnected = [this](int id) { _on_peer_disconnected(id); };
        #endif
        #if defined(Vital_SDK_Client)
        bridge->on_connected_to_server = [this]() { _on_connected_to_server(); };
        bridge->on_connection_failed   = [this]() { _on_connection_failed(); };
        bridge->on_server_disconnected = [this]() { _on_server_disconnected(); };
        #endif
    }

    void Network::destroy_bridge() {
        if (!bridge) return;
        memdelete(bridge);
        bridge = nullptr;
    }

    #if !defined(Vital_SDK_Client)
    void Network::wire_server_signals() {
        create_bridge();
        if (!bridge) return;
        auto mp = get_scene_tree()->get_multiplayer();
        if (!mp.is_valid()) return;
        mp->connect("peer_connected",    godot::Callable(bridge, "_on_peer_connected"));
        mp->connect("peer_disconnected", godot::Callable(bridge, "_on_peer_disconnected"));
    }
    #endif

    #if defined(Vital_SDK_Client)
    void Network::wire_client_signals() {
        create_bridge();
        if (!bridge) { Vital::print("sbox", "wire_client_signals: no bridge"); return; }
        auto mp = get_scene_tree()->get_multiplayer();
        if (!mp.is_valid()) { Vital::print("sbox", "wire_client_signals: mp invalid"); return; }
        Vital::print("sbox", "wire_client_signals: OK");
        mp->connect("connected_to_server", godot::Callable(bridge, "_on_connected_to_server"));
        mp->connect("connection_failed",   godot::Callable(bridge, "_on_connection_failed"));
        mp->connect("server_disconnected", godot::Callable(bridge, "_on_server_disconnected"));
    }
    #endif

    void Network::unwire_signals() {
        auto tree = get_scene_tree();
        if (!tree || !bridge) return;
        auto mp = tree->get_multiplayer();
        if (!mp.is_valid()) return;
        auto try_disconnect = [&](const char* signal, const char* method) {
            godot::Callable cb(bridge, method);
            if (mp->is_connected(signal, cb))
                mp->disconnect(signal, cb);
        };
        #if !defined(Vital_SDK_Client)
        try_disconnect("peer_connected",    "_on_peer_connected");
        try_disconnect("peer_disconnected", "_on_peer_disconnected");
        #endif
        #if defined(Vital_SDK_Client)
        try_disconnect("connected_to_server", "_on_connected_to_server");
        try_disconnect("connection_failed",   "_on_connection_failed");
        try_disconnect("server_disconnected", "_on_server_disconnected");
        #endif
    }


    //----------------//
    //     State      //
    //----------------//

    bool Network::is_active() const {
        return peer.is_valid() &&
            peer->get_connection_status() == godot::MultiplayerPeer::CONNECTION_CONNECTED;
    }

    bool Network::is_connecting() const {
        return peer.is_valid() &&
            peer->get_connection_status() == godot::MultiplayerPeer::CONNECTION_CONNECTING;
    }

    bool Network::is_server() {
        return get_platform() == "server";
    }

    int Network::get_peer_id() const {
        return peer.is_valid() ? peer->get_unique_id() : 0;
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


    //----------------//
    //     Server     //
    //----------------//

    #if !defined(Vital_SDK_Client)
    bool Network::host(int port, int max_clients) {
        if (!is_server()) {
            Vital::print("sbox", "Network: host() called on non-server");
            return false;
        }
        if (is_active()) {
            Vital::print("sbox", "Network: already hosting");
            return false;
        }
        peer.instantiate();
        godot::Error err = peer->create_server(port, max_clients);
        if (err != godot::OK) {
            Vital::print("sbox", "Network: failed to host on port ", port, " (err=", (int)err, ")");
            peer.unref();
            return false;
        }
        auto tree = get_scene_tree();
        if (!tree) { peer.unref(); return false; }
        tree->get_multiplayer()->set_multiplayer_peer(peer);
        wire_server_signals();
        Vital::print("sbox", "Network: hosting on port ", port);
        Vital::Tool::Event::emit("network:hosted", {});
        return true;
    }

    bool Network::close() {
        if (!peer.is_valid()) return false;
        unwire_signals();
        send_queue.clear();
        connected_peers.clear();
        peer->close();
        peer.unref();
        auto tree = get_scene_tree();
        if (tree) tree->get_multiplayer()->set_multiplayer_peer(nullptr);
        Vital::print("sbox", "Network: server closed");
        Vital::Tool::Event::emit("network:closed", {});
        return true;
    }

    void Network::_on_peer_connected(int id) {
        connected_peers.insert(id);
        Vital::print("sbox", "Network: peer joined -> ", id, "  total: ", (int)connected_peers.size());
        auto args = make_stack(static_cast<int32_t>(id));
        Vital::Tool::Event::emit("network:peer_joined", args);
    }

    void Network::_on_peer_disconnected(int id) {
        connected_peers.erase(id);
        Vital::print("sbox", "Network: peer left -> ", id, "  remaining: ", (int)connected_peers.size());
        auto args = make_stack(static_cast<int32_t>(id));
        Vital::Tool::Event::emit("network:peer_left", args);
    }

    const std::unordered_set<int>& Network::get_connected_peers() const {
        return connected_peers;
    }

    int Network::get_peer_count() const {
        return static_cast<int>(connected_peers.size());
    }
    #endif


    //----------------//
    //     Client     //
    //----------------//

    #if defined(Vital_SDK_Client)
    bool Network::connect_to_server(const std::string& ip, int port, bool enable_reconnect) {
        if (is_active() || is_connecting()) {
            Vital::print("sbox", "Network: already connected/connecting");
            return false;
        }
        peer.instantiate();
        if (peer->create_client(godot::String(ip.c_str()), port) != godot::OK) {
            Vital::print("sbox", "Network: failed to connect to ", ip.c_str(), ":", port);
            peer.unref();
            return false;
        }
        auto tree = get_scene_tree();
        if (!tree) { peer.unref(); return false; }
        tree->get_multiplayer()->set_multiplayer_peer(peer);
        wire_client_signals();
        auto_reconnect     = enable_reconnect;
        reconnect_ip       = ip;
        reconnect_port     = port;
        reconnect_attempts = 0;
        reconnect_timer    = 0.0f;
        pending_handshake  = false;
        send_queue.clear();
        Vital::print("sbox", "Network: connecting to ", ip.c_str(), ":", port);
        Vital::Tool::Event::emit("network:connecting", {});
        return true;
    }

    bool Network::disconnect_from_server() {
        if (!peer.is_valid()) return false;
        auto_reconnect    = false;
        pending_handshake = false;
        send_queue.clear();
        unwire_signals();
        peer->close();
        peer.unref();
        auto tree = get_scene_tree();
        if (tree) tree->get_multiplayer()->set_multiplayer_peer(nullptr);
        Vital::print("sbox", "Network: disconnected");
        Vital::Tool::Event::emit("network:disconnected", {});
        return true;
    }

    void Network::_on_connected_to_server() {
        reconnect_attempts = 0;
        pending_handshake  = true;
        Vital::print("sbox", "Network: connected (handshake deferred)");
        Vital::Tool::Event::emit("network:connected", {});
    }

    void Network::_on_connection_failed() {
        Vital::print("sbox", "Network: connection failed");
        pending_handshake = false;
        send_queue.clear();
        unwire_signals();
        if (peer.is_valid()) peer.unref();
        Vital::Tool::Event::emit("network:connection_failed", {});
        if (auto_reconnect) _schedule_reconnect();
    }

    void Network::_on_server_disconnected() {
        Vital::print("sbox", "Network: server dropped connection");
        pending_handshake = false;
        send_queue.clear();
        unwire_signals();
        if (peer.is_valid()) peer.unref();
        Vital::Tool::Event::emit("network:server_disconnected", {});
        if (auto_reconnect) _schedule_reconnect();
    }
    #endif


    //--------------------//
    //   Auto-Reconnect   //
    //--------------------//

    #if defined(Vital_SDK_Client)
    void Network::set_reconnect_config(int max_attempts, float delay_seconds) {
        reconnect_max   = max_attempts;
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
        Vital::print("sbox", "Network: retry in ", reconnect_delay,
            "s  attempt ", reconnect_attempts, "/", reconnect_max);
        Vital::Tool::Event::emit("network:reconnecting", {});
    }
    #endif


    //--------------------//
    //   Send / Receive   //
    //--------------------//

    bool Network::send(const Vital::Tool::Stack& stack, int peerID, bool isLatent) {
        if (!peer.is_valid()) return false;
        #if defined(Vital_SDK_Client)
        if (!is_active()) return false;
        #endif
        std::string bytes = stack.serialize();
        if (bytes.empty()) return false;
        godot::PackedByteArray packet;
        packet.resize(bytes.size());
        memcpy(packet.ptrw(), bytes.data(), bytes.size());
        peer->set_target_peer(peerID);
        peer->set_transfer_mode(isLatent
            ? godot::MultiplayerPeer::TRANSFER_MODE_UNRELIABLE
            : godot::MultiplayerPeer::TRANSFER_MODE_RELIABLE
        );
        peer->put_packet(packet);
        return true;
    }

    bool Network::queue_send(const Vital::Tool::Stack& stack, int peerID, bool isLatent) {
        if (!peer.is_valid()) return false;
        send_queue.push_back({ stack, peerID, isLatent });
        return true;
    }

    bool Network::broadcast(const Vital::Tool::Stack& stack, bool isLatent) {
        return send(stack, 0, isLatent);
    }

    bool Network::send_to_server(const Vital::Tool::Stack& stack, bool isLatent) {
        return send(stack, 1, isLatent);
    }


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
        #endif

        if (!peer.is_valid()) return;

        #if !defined(Vital_SDK_Client)
        if (!is_active()) return;
        #endif

        // Flush outgoing queue from previous tick BEFORE polling
        // This ensures packets queued inside packet handlers are sent
        // at the correct time without causing re-entrancy
        if (!send_queue.empty()) {
            auto queue = std::move(send_queue);
            send_queue.clear();
            for (auto& p : queue)
                send(p.stack, p.peerID, p.isLatent);
        }

        peer->poll();
        if (!peer.is_valid()) return;

        #if defined(Vital_SDK_Client)
        // Deferred handshake — send ping on first poll() after connection
        // This guarantees peer ID is fully assigned and ENet send buffer is ready
        if (pending_handshake && is_active()) {
            pending_handshake = false;
            Vital::print("sbox", "Network: sending deferred ping, peer_id=", get_peer_id());
            Vital::Tool::Stack msg;
            msg.array.push_back(Vital::Tool::StackValue(std::string("ping")));
            msg.object["type"] = Vital::Tool::StackValue(std::string("system"));
            send(msg, 1, false);
        }
        #endif

        int packet_count = peer->get_available_packet_count();
        for (int i = 0; i < packet_count; i++) {
            if (!peer.is_valid()) break;
            try {
                int32_t sender             = static_cast<int32_t>(peer->get_packet_peer());
                godot::PackedByteArray raw = peer->get_packet();
                if (raw.size() == 0) continue;

                Vital::Tool::Stack stack = Vital::Tool::Stack::deserialize(
                    reinterpret_cast<const char*>(raw.ptr()), raw.size()
                );
                stack.object["sender_id"] = Vital::Tool::StackValue(sender);
                Vital::Tool::Event::emit("network:packet", stack);
            }
            catch (const std::exception& e) { Vital::print("error", "Network: packet error: ", e.what()); }
            catch (...)                      { Vital::print("error", "Network: unknown packet error"); }
        }
    }


    //----------------//
    //  Emit outbound //
    //----------------//

    void Network::emit(Vital::Tool::Stack& arguments, int peerID, bool isLatent) {
        get_singleton()->send(arguments, peerID, isLatent);
    }
}