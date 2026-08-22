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
#include <Vital.sandbox/Engine/public/model.h>
#include <Vital.sandbox/API/utility/event.h>


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
            godot::Engine::get_singleton()->get_main_loop()
        );
    }

    void Network::teardown() {
        #if defined(VSDK_Client)
        singleton->disconnect_from_server();
        #else
        singleton->close();
        #endif
        singleton->unwire_signals();
        singleton->destroy();
        if (singleton->peer.is_valid()) {
            singleton->peer->close();
            singleton->peer.unref();
        }
    }


    //--------------------//
    //    Node Helpers    //
    //--------------------//

    void Network::create() {
        if (node) return;
        node = memnew(Engine::Network);
        node->set_name("VitalNetwork");
        Engine::Core::get_scene_root()->add_child(node);
        node->setup_rpc();
        #if defined(VSDK_Client)
            node->on_connected_to_server = [this]() { _on_connected_to_server(); };
            node->on_connection_failed   = [this]() { _on_connection_failed(); };
            node->on_server_disconnected = [this]() { _on_server_disconnected(); };
        #else
            node->on_peer_connected    = [this](int id) { _on_peer_connected(id); };
            node->on_peer_disconnected = [this](int id) { _on_peer_disconnected(id); };
        #endif
    }

    void Network::destroy() {
        if (!node) return;
        node->queue_free();
        node = nullptr;
    }

    void Network::wire_signals() {
        if (!node) return;
        auto mp = get_scene_tree()->get_multiplayer();
        if (!mp.is_valid()) return;
        #if defined(VSDK_Client)
            mp->connect("connected_to_server", godot::Callable(node, "_on_connected_to_server"));
            mp->connect("connection_failed",   godot::Callable(node, "_on_connection_failed"));
            mp->connect("server_disconnected", godot::Callable(node, "_on_server_disconnected"));
        #else
            mp->connect("peer_connected",    godot::Callable(node, "_on_peer_connected"));
            mp->connect("peer_disconnected", godot::Callable(node, "_on_peer_disconnected"));
        #endif
    }

    void Network::unwire_signals() {
        auto tree = get_scene_tree();
        if (!tree || !node) return;
        auto mp = tree->get_multiplayer();
        if (!mp.is_valid()) return;
        auto try_disconnect = [&](const char* signal, const char* method) {
            godot::Callable cb(node, method);
            if (mp->is_connected(signal, cb)) mp->disconnect(signal, cb);
        };
        #if defined(VSDK_Client)
            try_disconnect("connected_to_server", "_on_connected_to_server");
            try_disconnect("connection_failed",   "_on_connection_failed");
            try_disconnect("server_disconnected", "_on_server_disconnected");
        #else
            try_disconnect("peer_connected",    "_on_peer_connected");
            try_disconnect("peer_disconnected", "_on_peer_disconnected");
        #endif
    }


    //----------------------------//
    //    Model Sync Registry     //
    //----------------------------//

    void Network::register_model(Engine::Model* model) {
        std::lock_guard<std::mutex> lock(sync_models_mutex);
        sync_models.push_back(model);
    }

    void Network::unregister_model(Engine::Model* model) {
        std::lock_guard<std::mutex> lock(sync_models_mutex);
        sync_models.erase(
            std::remove(sync_models.begin(), sync_models.end(), model),
            sync_models.end()
        );
    }


    //----------------//
    //     State      //
    //----------------//

    bool Network::is_connected() const {
        return peer.is_valid() &&
               peer->get_connection_status() == godot::MultiplayerPeer::CONNECTION_CONNECTED;
    }

    bool Network::is_connecting() const {
        return peer.is_valid() &&
               peer->get_connection_status() == godot::MultiplayerPeer::CONNECTION_CONNECTING;
    }

    int Network::get_peer_id() const {
        auto tree = get_scene_tree();
        if (!tree) return 0;
        auto mp = tree->get_multiplayer();
        return mp.is_valid() ? mp->get_unique_id() : 0;
    }


    //--------------------//
    //  Packet Received   //
    //--------------------//

    void Network::_on_packet_received(godot::Dictionary data) {
        auto tree = get_scene_tree();
        if (!tree) return;
        auto mp = tree->get_multiplayer();
        int32_t sender = mp.is_valid() ? mp->get_remote_sender_id() : 0;

        godot::Dictionary obj = data.has("object") ? (godot::Dictionary)data["object"] : godot::Dictionary();
        obj["sender_id"] = (int64_t)sender;
        data["object"] = obj;

        Tool::Stack stack = Tool::Stack::from_dict(data);

        if (stack.has("__event") || stack.has("__reply_serial")) {
            auto vm = Manager::Sandbox::get_singleton()->get_vm();
            Vital::Sandbox::API::Event::dispatch_remote(vm, stack);
            return;
        }

        Tool::Event::emit("network:packet", stack);
    }


    //-----------------------------------//
    //   Low-level Sync Transport        //
    //-----------------------------------//
    //
    // We bypass Godot's RPC layer for sync packets and talk directly
    // to ENetMultiplayerPeer::put_packet().  This lets us:
    //   • Pick TRANSFER_MODE_UNRELIABLE_ORDERED (drop-on-lag, no head-of-line blocking)
    //   • Use a dedicated channel (CHANNEL_SYNC = 1) so sync never delays RPC
    //   • Keep packets tiny (28 bytes) — no JSON/dictionary overhead
    //
    // Server flow (server-auth model):
    //   server calls broadcast_sync() -> peer->set_target_peer(0) -> put_packet
    //
    // Client-auth flow:
    //   authority client calls send_sync_to_server() -> peer->set_target_peer(1) -> put_packet
    //   server receives in drain_sync_packets(), finds the model, relays to all others
    //
    // In both cases non-authority peers call Model::apply_sync() which snaps
    // the transform. Scripters can layer interpolation on top.
    //-----------------------------------//

    bool Network::send_raw(const godot::PackedByteArray& data, int peerID, bool unreliable) {
        if (!peer.is_valid()) return false;
        peer->set_transfer_channel(CHANNEL_SYNC);
        peer->set_transfer_mode(unreliable
            ? godot::MultiplayerPeer::TRANSFER_MODE_UNRELIABLE_ORDERED
            : godot::MultiplayerPeer::TRANSFER_MODE_RELIABLE);
        peer->set_target_peer(peerID);
        peer->put_packet(data);
        return true;
    }

    bool Network::broadcast_sync(const godot::PackedByteArray& data) {
        #if defined(VSDK_Client)
        return false; // clients never broadcast; they send to server
        #else
        if (!is_connected()) return false;
        return send_raw(data, 0 /* broadcast */, true);
        #endif
    }

    bool Network::send_sync_to_server(const godot::PackedByteArray& data) {
        #if !defined(VSDK_Client)
        return false; // server doesn't send to itself
        #else
        if (!is_connected()) return false;
        return send_raw(data, 1 /* server */, true);
        #endif
    }

    // Drain all pending raw packets on CHANNEL_SYNC and route them.
    // Called every frame from poll().
    void Network::drain_sync_packets() {
        if (!peer.is_valid()) return;

        // Build a quick lookup from net_id -> Model* for O(1) dispatch.
        // We rebuild it each frame because the registry can change.
        // For 200 models this is negligible (< 1 µs).
        std::unordered_map<uint32_t, Engine::Model*> id_map;
        {
            std::lock_guard<std::mutex> lock(sync_models_mutex);
            id_map.reserve(sync_models.size());
            for (auto* m : sync_models) id_map[m->get_net_id()] = m;
        }

        #if !defined(VSDK_Client)
        // Server: drain client-authority sync packets and relay to all other peers.
        //
        // Packet layout from client (32 bytes):
        //   [0..3]   uint32  sender_peer_id  — client stamps its own peer ID
        //   [4..7]   uint32  net_id
        //   [8..11]  float   pos.x
        //   [12..15] float   pos.y
        //   [16..19] float   pos.z
        //   [20..23] float   rot.x
        //   [24..27] float   rot.y
        //   [28..31] float   rot.z
        //
        // The server reads sender_peer_id from the payload (no get_remote_sender_id
        // on ENetMultiplayerPeer — that API lives on MultiplayerAPI and is only
        // valid during RPC dispatch). The server validates the sender is actually
        // the registered authority before applying/relaying.
        while (peer->get_available_packet_count() > 0) {
            godot::PackedByteArray pkt = peer->get_packet();
            if (pkt.size() < 32) continue;

            uint32_t sender_id = Engine::Model::read_u32_public(pkt, 0);

            // Validate sender is a connected peer (basic anti-spoof).
            if (connected_peers.find((int)sender_id) == connected_peers.end()) continue;

            uint32_t net_id = 0;
            godot::Vector3 pos, rot;
            if (!Engine::Model::parse_sync_packet_at(pkt, 4, net_id, pos, rot)) continue;

            auto it = id_map.find(net_id);
            if (it == id_map.end()) continue;
            Engine::Model* model = it->second;

            // Only honour if the sender is the registered authority.
            if (model->get_sync_authority() != (int)sender_id) continue;

            // Apply locally — server holds ground truth.
            model->apply_sync(pos, rot);

            // Build a stripped 28-byte relay packet (drop sender prefix) and
            // broadcast to all other peers.
            godot::PackedByteArray relay = pkt.slice(4);
            for (int pid : connected_peers) {
                if (pid == (int)sender_id) continue;
                send_raw(relay, pid, true);
            }
        }
        #else
        // Client: drain packets from server.
        // Two packet formats:
        //   State dump (magic 0x56535354) — batched reliable snapshot for late-joiners.
        //   Normal 28-byte sync packet    — per-model unreliable tick.
        int my_id = get_peer_id();

        auto apply_one = [&](const godot::PackedByteArray& pkt, int offset) {
            uint32_t net_id = 0;
            godot::Vector3 pos, rot;
            if (!Engine::Model::parse_sync_packet_at(pkt, offset, net_id, pos, rot)) return;
            auto it = id_map.find(net_id);
            if (it == id_map.end()) return;
            Engine::Model* model = it->second;
            if (model->get_sync_authority() == my_id) return;
            model->apply_sync(pos, rot);
        };

        while (peer->get_available_packet_count() > 0) {
            godot::PackedByteArray pkt = peer->get_packet();
            if (pkt.size() < 4) continue;

            uint32_t magic = (uint8_t)pkt[0]
                           | ((uint8_t)pkt[1] << 8)
                           | ((uint8_t)pkt[2] << 16)
                           | ((uint8_t)pkt[3] << 24);

            if (magic == STATE_DUMP_MAGIC) {
                // Batched late-join state dump.
                if (pkt.size() < 8) continue;
                uint32_t count = (uint8_t)pkt[4]
                               | ((uint8_t)pkt[5] << 8)
                               | ((uint8_t)pkt[6] << 16)
                               | ((uint8_t)pkt[7] << 24);
                if ((int)pkt.size() < 8 + (int)count * 28) continue;
                for (uint32_t i = 0; i < count; i++) {
                    apply_one(pkt, 8 + (int)i * 28);
                }
            } else {
                // Normal unreliable sync tick.
                apply_one(pkt, 0);
            }
        }
        #endif
    }


    //------------------//
    //  Client Methods  //
    //------------------//

    #if defined(VSDK_Client)
    bool Network::connect_to_server(const std::string& ip, int port, bool enable_reconnect) {
        if (is_connected() || is_connecting()) {
            log("sbox", "already connected/connecting");
            return false;
        }
        create();
        peer.instantiate();

        // ENet channel count: 2 (RPC + SYNC)
        godot::Error err = peer->create_client(godot::String(ip.c_str()), port,
                                                0, 0, 0, 2 /* channel_count */);
        if (err != godot::OK) {
            log("sbox", fmt::format("failed to connect to {}:{}", ip, port));
            peer.unref();
            return false;
        }
        auto tree = get_scene_tree();
        if (!tree) { peer.unref(); return false; }
        tree->get_multiplayer()->set_multiplayer_peer(peer);
        wire_signals();
        auto_reconnect    = enable_reconnect;
        reconnect_ip      = ip;
        reconnect_port    = port;
        reconnect_attempts = 0;
        reconnect_timer   = 0.0f;
        pending_handshake = false;
        log("sbox", fmt::format("connecting to {}:{}", ip, port));
        Tool::Event::emit("network:connect", {});
        return true;
    }

    bool Network::disconnect_from_server() {
        if (!peer.is_valid()) return false;
        auto_reconnect    = false;
        pending_handshake = false;
        unwire_signals();
        peer->close();
        peer.unref();
        auto tree = get_scene_tree();
        if (tree) tree->get_multiplayer()->set_multiplayer_peer(nullptr);
        log("sbox", "disconnected");
        Tool::Event::emit("network:disconnect", {});
        return true;
    }

    void Network::_on_connected_to_server() {
        reconnect_attempts = 0;
        pending_handshake  = true;
        log("sbox", "connected (handshake deferred)");
        Tool::Event::emit("network:connect:success", {});
    }

    void Network::_on_connection_failed() {
        log("sbox", "connection failed");
        pending_handshake = false;
        unwire_signals();
        if (peer.is_valid()) peer.unref();
        Tool::Event::emit("network:connect:failed", {});
        if (auto_reconnect) _schedule_reconnect();
    }

    void Network::_on_server_disconnected() {
        log("sbox", "server dropped connection");
        pending_handshake = false;
        unwire_signals();
        if (peer.is_valid()) peer.unref();
        Tool::Event::emit("network:server:disconnect", {});
        if (auto_reconnect) _schedule_reconnect();
    }

    void Network::set_reconnect_config(int max_attempts, float delay_seconds) {
        reconnect_max   = max_attempts;
        reconnect_delay = delay_seconds;
    }

    void Network::_schedule_reconnect() {
        if (reconnect_attempts >= reconnect_max) {
            log("sbox", "max reconnect attempts reached");
            auto_reconnect = false;
            Tool::Event::emit("network:reconnect:failed", {});
            return;
        }
        reconnect_attempts++;
        reconnect_timer = reconnect_delay;
        log("sbox", fmt::format("retry in {}s  attempt {}/{}", reconnect_delay, reconnect_attempts, reconnect_max));
        Tool::Event::emit("network:reconnect", {});
    }

    std::string Network::get_server_ip() const { return reconnect_ip; }


    //------------------//
    //  Server Methods  //
    //------------------//

    #else
    bool Network::host(Config::Server& config) {
        if (is_connected()) {
            log("sbox", "already hosting");
            return false;
        }

        const int net_port = config.get_network_port();
        {
            godot::Ref<godot::UDPServer> probe;
            probe.instantiate();
            if (probe->listen(net_port) != godot::OK) {
                log("error", fmt::format("failed to host on port {} — already in use", net_port));
                return false;
            }
            probe->stop();
        }

        server_config = &config;
        create();
        peer.instantiate();

        // ENet channel count: 2 (RPC + SYNC)
        godot::Error err = peer->create_server(net_port, config.get_max_clients(),
                                                0, 0, 2 /* channel_count */);
        if (err != godot::OK) {
            log("sbox", fmt::format("failed to host on port {} (err={})", net_port, (int)err));
            peer.unref();
            server_config = nullptr;
            return false;
        }
        auto tree = get_scene_tree();
        if (!tree) { peer.unref(); server_config = nullptr; return false; }
        tree->get_multiplayer()->set_multiplayer_peer(peer);
        wire_signals();
        try {
            server_ip = Tool::HTTP::get("https://api.ipify.org", {}, 10);
            if (!server_ip.empty() && std::isspace((unsigned char)server_ip.back()))
                server_ip.pop_back();
        }
        catch (...) {}
        log("sbox", fmt::format(
            "Server is live!\n"
            "> IP — `{}`\n"
            "> Port — `{}`",
            get_server_ip(),
            get_server_config().get_network_port()
        ));
        Tool::Event::emit("network:host", {});
        return true;
    }

    bool Network::close() {
        if (!peer.is_valid()) return false;
        unwire_signals();
        connected_peers.clear();
        peer->close();
        peer.unref();
        auto tree = get_scene_tree();
        if (tree) tree->get_multiplayer()->set_multiplayer_peer(nullptr);
        log("sbox", "server closed");
        Tool::Event::emit("network:close", {});
        return true;
    }

    // Full state dump for late-joiners.
    //
    // Packet layout for each model — identical to the normal sync packet (28 bytes)
    // but sent on CHANNEL_RPC (reliable) so it's guaranteed to arrive after the
    // MultiplayerSpawner spawn signals that Godot emits for the same peer.
    //
    // We batch all models into a single reliable send via a small framing header:
    //   [0..3]   uint32  magic  0x56535354 ("VSST" — Vital Sync State Table)
    //   [4..7]   uint32  count  number of model entries that follow
    //   [8..]    N * 28 bytes   one sync packet per model
    //
    // The client's drain_sync_packets() recognises the magic and unpacks all entries
    // in one pass, so there is no per-model packet overhead on the receiving side.
    //
    void Network::send_full_state_to_peer(int peer_id) {
        if (!peer.is_valid() || !is_connected()) return;

        std::vector<Engine::Model*> snapshot;
        {
            std::lock_guard<std::mutex> lock(sync_models_mutex);
            snapshot = sync_models; // copy under lock, work outside
        }

        if (snapshot.empty()) return;

        // Build the batch buffer.
        const uint32_t count = static_cast<uint32_t>(snapshot.size());
        godot::PackedByteArray buf;
        buf.resize(8 + count * 28);

        // Write header.
        auto wu32 = [&](int off, uint32_t v) {
            buf[off]   =  v        & 0xFF;
            buf[off+1] = (v >>  8) & 0xFF;
            buf[off+2] = (v >> 16) & 0xFF;
            buf[off+3] = (v >> 24) & 0xFF;
        };
        auto wf32 = [&](int off, float v) {
            uint32_t raw; memcpy(&raw, &v, 4); wu32(off, raw);
        };

        wu32(0, STATE_DUMP_MAGIC);
        wu32(4, count);

        int offset = 8;
        for (auto* model : snapshot) {
            godot::Vector3 pos = model->get_global_position();
            godot::Vector3 rot = model->get_rotation_degrees();
            wu32(offset,      model->get_net_id());
            wf32(offset +  4, pos.x);
            wf32(offset +  8, pos.y);
            wf32(offset + 12, pos.z);
            wf32(offset + 16, rot.x);
            wf32(offset + 20, rot.y);
            wf32(offset + 24, rot.z);
            offset += 28;
        }

        // Send reliable (channel 0) unicast to the joining peer.
        peer->set_transfer_channel(CHANNEL_RPC);
        peer->set_transfer_mode(godot::MultiplayerPeer::TRANSFER_MODE_RELIABLE);
        peer->set_target_peer(peer_id);
        peer->put_packet(buf);

        log("sbox", fmt::format("state dump -> peer {}  ({} models, {} bytes)",
            peer_id, count, buf.size()));
    }

    void Network::_on_peer_connected(int id) {
        connected_peers.insert(id);
        log("sbox", fmt::format("peer joined -> {}  total: {}", id, (int)connected_peers.size()));

        // Send the full world state so the late-joiner snaps to correct
        // transforms immediately — even for sleeping (static) models that
        // won't emit another sync tick until they move again.
        send_full_state_to_peer(id);

        Tool::Stack args;
        args.array.push_back(Tool::StackValue((int32_t)id));
        Tool::Event::emit("network:peer:join", args);
    }

    void Network::_on_peer_disconnected(int id) {
        connected_peers.erase(id);
        log("sbox", fmt::format("peer left -> {}  remaining: {}", id, (int)connected_peers.size()));
        Tool::Stack args;
        args.array.push_back(Tool::StackValue((int32_t)id));
        Tool::Event::emit("network:peer:leave", args);
    }

    const std::unordered_set<int>& Network::get_connected_peers() const { return connected_peers; }
    int  Network::get_peer_count() const { return static_cast<int>(connected_peers.size()); }
    const Config::Server& Network::get_server_config() const { return *server_config; }
    std::string Network::get_server_ip() const { return server_ip; }
    #endif


    //--------------------//
    //   Send / Receive   //
    //--------------------//

    bool Network::send(const Tool::Stack& stack, int peerID) {
        if (!node || !peer.is_valid()) return false;
        #if defined(VSDK_Client)
        if (!is_connected()) return false;
        #endif
        if (peerID == 0) node->rpc("_receive", stack.to_dict());
        else             node->rpc_id(peerID, "_receive", stack.to_dict());
        return true;
    }

    bool Network::broadcast(const Tool::Stack& stack)      { return send(stack, 0); }
    bool Network::send_to_server(const Tool::Stack& stack) { return send(stack, 1); }


    //----------//
    //   Poll   //
    //----------//

    void Network::poll(double delta) {
        #if defined(VSDK_Client)
        // Reconnect timer
        if (auto_reconnect && !is_connected() && !is_connecting()) {
            if (reconnect_timer > 0.0f) {
                reconnect_timer -= static_cast<float>(delta);
                if (reconnect_timer <= 0.0f)
                    connect_to_server(reconnect_ip, reconnect_port, true);
            }
            return;
        }
        // Handshake
        if (pending_handshake && is_connected()) {
            pending_handshake = false;
            log("sbox", fmt::format("sending handshake, peer_id={}", get_peer_id()));
            Tool::Stack msg;
            msg.array.push_back(Tool::StackValue(std::string("ping")));
            msg.object["event"] = Tool::StackValue(std::string("system"));
            send_to_server(msg);
        }
        #endif

        // Lazy registration: any Model that is now inside the scene tree
        // but hasn't been registered yet (because _ready fired during create()
        // before we had a chance to register it) gets picked up here.
        // This runs on the main thread inside poll() — always safe.
        {
            auto core = Engine::Core::get_singleton();
            if (core) {
                for (int i = 0; i < core->get_child_count(); i++) {
                    auto* model = godot::Object::cast_to<Engine::Model>(core->get_child(i));
                    if (model && !model->sync_registered && model->is_inside_tree()) {
                        std::lock_guard<std::mutex> lock(sync_models_mutex);
                        sync_models.push_back(model);
                        model->sync_registered = true;
                    }
                }
            }
        }

        // Drive all registered models' sync tick.
        {
            std::lock_guard<std::mutex> lock(sync_models_mutex);
            for (auto* model : sync_models) {
                model->sync_tick(static_cast<float>(delta));
            }
        }

        // Drain inbound raw sync packets and dispatch.
        drain_sync_packets();
    }
}
