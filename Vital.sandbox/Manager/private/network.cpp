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
#include <Vital.sandbox/Engine/public/syncable.h>
#include <Vital.sandbox/Engine/public/model.h>
#include <Vital.sandbox/API/utility/event.h>


//////////////////////////////
// Vital: Manager: Network //
//////////////////////////////

// TOOD: Improve
namespace Vital::Manager {

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

    void Network::register_syncable(Engine::ISyncable* entity) {
        std::lock_guard<std::mutex> lock(sync_models_mutex);
        sync_models.push_back(entity);
        sync_id_map[entity->get_net_id()] = entity;
    }

    void Network::unregister_syncable(Engine::ISyncable* entity) {
        std::lock_guard<std::mutex> lock(sync_models_mutex);
        sync_models.erase(
            std::remove(sync_models.begin(), sync_models.end(), entity),
            sync_models.end()
        );
        sync_id_map.erase(entity->get_net_id());
    }

    // Posts to the pending queue — safe to call from any thread/enqueue context.
    void Network::enqueue_syncable_registration(Engine::ISyncable* entity) {
        std::lock_guard<std::mutex> lock(sync_pending_mutex);
        sync_pending.push_back(entity);
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
    // Sync is routed through Godot's RPC layer (_sync_models / _sync_client / _sync_state)
    // routed through Godot's RPC layer (avoids scene_cache_interface conflicts).

    bool Network::broadcast_sync(const godot::PackedByteArray& data) {
        #if defined(VSDK_Client)
        return false;
        #else
        if (!node || !is_connected()) return false;
        node->rpc("_sync_entities", data);
        return true;
        #endif
    }

    bool Network::send_sync_to_server(const godot::PackedByteArray& data) {
        #if !defined(VSDK_Client)
        return false;
        #else
        if (!node || !is_connected()) return false;
        node->rpc_id(1, "_sync_client", data);
        return true;
        #endif
    }

    // Called by Engine::Network::_sync_models (unreliable) and _sync_state (reliable).
    // All sync packets now use VSST batch format — no single-model packets.
    void Network::dispatch_sync_batch(const godot::PackedByteArray& data, bool /*is_state_dump*/) {
        if (data.size() < 8) return;
        if (Engine::ISyncable::read_u32_public(data, 0) != STATE_DUMP_MAGIC) return;
        uint32_t payload_bytes = Engine::ISyncable::read_u32_public(data, 4);
        if ((int)data.size() < 8 + (int)payload_bytes) return;

        int my_id = get_peer_id();
        std::lock_guard<std::mutex> lock(sync_models_mutex);

        int offset = 8;
        int end    = 8 + (int)payload_bytes;
        while (offset < end) {
            uint32_t net_id = 0;
            godot::Vector3 pos, rot, vel;

            auto it_model = sync_id_map.end();
            // Peek net_id to find model before decoding (need delta_last_* from model).
            if (offset + 4 > end) break;
            net_id = Engine::ISyncable::read_u32_public(data, offset);
            it_model = sync_id_map.find(net_id);

            Engine::ISyncable* model = (it_model != sync_id_map.end()) ? it_model->second : nullptr;
            int consumed;
            if (model) {
                consumed = model->parse_sync_packet_at(data, offset, net_id, pos, rot, vel);
            } else {
                // Unknown model — skip this entry using throwaway state.
                godot::Vector3 dp, dr, dv;
                consumed = Engine::ISyncable::decode_delta_public(data, offset, (int)data.size(),
                    net_id, pos, rot, vel, dp, dr, dv);
            }
            if (consumed < 0) break;
            offset += consumed;

            if (!model) continue;
            if (model->get_sync_authority() == my_id) continue;
            model->apply_sync(pos, rot, vel);
        }
    }

    // Called by Engine::Network::_sync_client — batched client-auth upload on server.
    // Client packet layout: [sender u32][VSST magic u32][count u32][N*28 bytes]
    void Network::dispatch_client_sync(const godot::PackedByteArray& data, int sender_id) {
        #if !defined(VSDK_Client)
        if (data.size() < 12) return;
        if (connected_peers.find(sender_id) == connected_peers.end()) return;
        if ((int)Engine::ISyncable::read_u32_public(data, 0) != sender_id) return; // anti-spoof
        if (Engine::ISyncable::read_u32_public(data, 4) != STATE_DUMP_MAGIC) return;

        uint32_t payload_bytes = Engine::ISyncable::read_u32_public(data, 8);
        if ((int)data.size() < 12 + (int)payload_bytes) return;

        // Relay buffer: strip sender prefix, keep VSST header + variable entries.
        godot::PackedByteArray relay;
        relay.resize(8 + (int)payload_bytes); // max size, may shrink
        auto wu32_r = [&](int off, uint32_t v) {
            relay[off]   =  v        & 0xFF;
            relay[off+1] = (v >>  8) & 0xFF;
            relay[off+2] = (v >> 16) & 0xFF;
            relay[off+3] = (v >> 24) & 0xFF;
        };
        wu32_r(0, STATE_DUMP_MAGIC);

        int relay_cursor = 8;
        {
            std::lock_guard<std::mutex> lock(sync_models_mutex);
            int offset = 12;
            int end    = 12 + (int)payload_bytes;
            while (offset < end) {
                if (offset + 4 > end) break;
                uint32_t net_id = Engine::ISyncable::read_u32_public(data, offset);
                godot::Vector3 pos, rot, vel;

                auto it = sync_id_map.find(net_id);
                Engine::ISyncable* model = (it != sync_id_map.end()) ? it->second : nullptr;

                int consumed;
                if (model) {
                    consumed = model->parse_sync_packet_at(data, offset, net_id, pos, rot, vel);
                } else {
                    godot::Vector3 dp, dr, dv;
                    consumed = Engine::ISyncable::decode_delta_public(data, offset, (int)data.size(),
                        net_id, pos, rot, vel, dp, dr, dv);
                }
                if (consumed < 0) break;

                if (model && model->get_sync_authority() == sender_id) {
                    model->apply_sync(pos, rot, vel);
                    // Copy variable-length entry verbatim into relay.
                    for (int b = 0; b < consumed; b++)
                        relay[relay_cursor + b] = data[offset + b];
                    relay_cursor += consumed;
                }
                offset += consumed;
            }
        }

        if (relay_cursor > 8 && node) {
            relay.resize(relay_cursor);
            wu32_r(4, (uint32_t)(relay_cursor - 8));
            for (int pid : connected_peers) {
                if (pid == sender_id) continue;
                node->rpc_id(pid, "_sync_entities", relay);
            }
        }
        #endif
    }


    Engine::ISyncable* Network::find_syncable(uint32_t net_id) {
        std::lock_guard<std::mutex> lock(sync_models_mutex);
        auto it = sync_id_map.find(net_id);
        return (it != sync_id_map.end()) ? it->second : nullptr;
    }


    //------------------//
    //  Client Methods  //    //------------------//
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

        // ENet with default channel count (RPC via MultiplayerAPI)
        godot::Error err = peer->create_client(godot::String(ip.c_str()), port,
                                                0, 0, 0);
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

        // ENet with default channel count (RPC via MultiplayerAPI)
        godot::Error err = peer->create_server(net_port, config.get_max_clients(),
                                                0, 0, 0);
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
        sync_interval = 1.0f / static_cast<float>(config.get_sync_rate());

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
    // but sent reliably so it's guaranteed to arrive after the
    // spawn RPCs that Godot emits for the same peer.
    //
    // We batch all models into a single reliable send via a small framing header:
    //   [0..3]   uint32  magic  0x56535354 ("VSST" — Vital Sync State Table)
    //   [4..7]   uint32  count  number of model entries that follow
    //   [8..]    variable-length delta entries (6 + N*4 bytes each)
    //
    // The client's dispatch_sync_batch() recognises the magic and unpacks all entries
    // in one pass, so there is no per-model packet overhead on the receiving side.
    //
    void Network::send_full_state_to_peer(int peer_id) {
        if (!peer.is_valid() || !is_connected()) return;

        // sync_pending was already flushed by _on_peer_connected before this call.
        std::vector<Engine::ISyncable*> snapshot;
        {
            std::lock_guard<std::mutex> lock(sync_models_mutex);
            snapshot = sync_models;
        }

        if (snapshot.empty()) return;

        // Build the batch buffer.
        const uint32_t count = static_cast<uint32_t>(snapshot.size());
        godot::PackedByteArray buf;
        buf.resize(8 + count * Engine::ISyncable::SYNC_PACKET_MAX);

        auto wu32 = [&](int off, uint32_t v) {
            buf[off]   =  v        & 0xFF;
            buf[off+1] = (v >>  8) & 0xFF;
            buf[off+2] = (v >> 16) & 0xFF;
            buf[off+3] = (v >> 24) & 0xFF;
        };

        wu32(0, STATE_DUMP_MAGIC);
        // [4..7] will be filled with payload_bytes after encoding.

        // Late-joiners have no delta state — force all components by using
        // zeroed last_* so encode_delta always sets all 9 bits in the mask.
        int cursor = 8;
        for (auto* model : snapshot) {
            godot::Vector3 pos = model->get_sync_position();
            godot::Vector3 rot = model->get_sync_rotation();
            godot::Vector3 vel = model->sync_last_vel;
            // Temp zeroed state — guarantees full packet (all bits set).
            godot::Vector3 zero_p, zero_r, zero_v;
            int written = Engine::ISyncable::encode_delta_public(
                buf, cursor,
                model->get_net_id(), pos, rot, vel,
                zero_p, zero_r, zero_v);
            cursor += written;
        }
        buf.resize(cursor);
        wu32(4, (uint32_t)(cursor - 8)); // payload_bytes

        // Send reliable unicast to the joining peer via RPC.
        if (node) node->rpc_id(peer_id, "_sync_state", buf);

        log("sbox", fmt::format("state dump -> peer {}  ({} models, {} bytes)",
            peer_id, count, buf.size()));
    }

    void Network::_on_peer_connected(int id) {
        connected_peers.insert(id);
        log("sbox", fmt::format("peer joined -> {}  total: {}", id, (int)connected_peers.size()));

        // 1. Flush pending so newly created models are in sync_models before we iterate.
        //    (send_full_state_to_peer does this too, but we need the list for spawns first.)
        {
            std::vector<Engine::ISyncable*> incoming;
            {
                std::lock_guard<std::mutex> lock(sync_pending_mutex);
                incoming.swap(sync_pending);
            }
            if (!incoming.empty()) {
                std::lock_guard<std::mutex> lock(sync_models_mutex);
                for (auto* m : incoming) {
                    if (!m->sync_registered) {
                        sync_models.push_back(m);
                        sync_id_map[m->get_net_id()] = m; // ISyncable*
                        m->sync_registered = true;
                    }
                }
            }
        }

        // 2. Send _spawn_model for every existing model so the late-joiner
        //    creates the nodes before the transform state dump arrives.
        if (node) {
            std::lock_guard<std::mutex> lock(sync_models_mutex);
            for (auto* e : sync_models) {
                node->rpc_id(id, "_spawn_entity",
                    (int)e->get_net_id(),
                    (int)e->get_sync_type(),
                    Tool::to_godot_string(e->get_sync_name()),
                    e->get_sync_authority());
            }
        }

        // 3. Send transform state dump (reliable) so models snap to correct positions.
        send_full_state_to_peer(id);

        Tool::Stack args;
        args.array.push_back(Tool::StackValue((int32_t)id));
        Tool::Event::emit("network:peer:join", args);
    }

    void Network::_on_peer_disconnected(int id) {
        connected_peers.erase(id);
        log("sbox", fmt::format("peer left -> {}  remaining: {}", id, (int)connected_peers.size()));

        // Auto-revoke: any model owned by this peer falls back to server authority.
        // set_syncer(1) also broadcasts _set_authority to all remaining clients.
        {
            std::vector<Engine::ISyncable*> snapshot;
            {
                std::lock_guard<std::mutex> lock(sync_models_mutex);
                snapshot = sync_models;
            }
            for (auto* entity : snapshot) {
                if (entity->get_sync_authority() == id) {
                    // Revert to server authority and broadcast to all remaining clients.
                    entity->sync_authority = 1;
                    entity->sync_sleeping  = false;
                    if (node) node->rpc("_set_authority", (int)entity->get_net_id(), 1);
                    log("sbox", fmt::format("auto-revoke: net_id={} -> server (peer {} disconnected)",
                        entity->get_net_id(), id));
                }
            }
        }

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

        // Flush the pending registration queue — models posted via
        // enqueue_syncable_registration() from deferred add_child callbacks.
        // O(pending) not O(all children), so it scales cleanly.
        {
            std::vector<Engine::ISyncable*> incoming;
            {
                std::lock_guard<std::mutex> lock(sync_pending_mutex);
                incoming.swap(sync_pending);
            }
            if (!incoming.empty()) {
                std::lock_guard<std::mutex> lock(sync_models_mutex);
                for (auto* entity : incoming) {
                    if (!entity->sync_registered) {
                        sync_models.push_back(entity);
                        sync_id_map[entity->get_net_id()] = entity;
                        entity->sync_registered = true;
                        entity->interp_step = sync_interval;
                    }
                }
            }
        }

        // ── Sync batch — one VSST packet per frame, broadcast to all peers ──────
        #if !defined(VSDK_Client)
        {
            std::vector<Engine::ISyncable*> snapshot;
            {
                std::lock_guard<std::mutex> lock(sync_models_mutex);
                snapshot = sync_models;
            }
            if (snapshot.empty() || !node || !is_connected()) return;

            // Delta batch: [VSST magic u32][payload_bytes u32][variable entries]
            sync_batch_buf.resize(8 + (int)snapshot.size() * Engine::ISyncable::SYNC_PACKET_MAX);
            int cursor = 8;

            auto wu32 = [&](int off, uint32_t v) {
                sync_batch_buf[off]   =  v        & 0xFF;
                sync_batch_buf[off+1] = (v >>  8) & 0xFF;
                sync_batch_buf[off+2] = (v >> 16) & 0xFF;
                sync_batch_buf[off+3] = (v >> 24) & 0xFF;
            };

            for (auto* model : snapshot) {
                if (!model->is_sync_active()) continue;
                if (model->get_sync_authority() != 1) continue;

                godot::Vector3 cur_pos = model->get_sync_position();
                godot::Vector3 cur_rot = model->get_sync_rotation();

                bool moved = (cur_pos - model->sync_last_pos).length() > 0.001f
                          || (cur_rot - model->sync_last_rot).length() > 0.001f;

                if (!moved) {
                    if (model->sync_sleeping) continue;
                    model->sync_sleeping = true;
                } else {
                    model->sync_sleeping = false;
                }

                model->sync_accum += static_cast<float>(delta);
                if (model->sync_accum < sync_interval && !model->sync_sleeping) continue;

                godot::Vector3 cur_vel = (cur_pos - model->sync_last_pos) / sync_interval;
                model->sync_accum    = 0.0f;
                model->sync_last_pos = cur_pos;
                model->sync_last_rot = cur_rot;
                model->sync_last_vel = cur_vel;

                int written = Engine::ISyncable::encode_delta_public(
                    sync_batch_buf, cursor,
                    model->get_net_id(), cur_pos, cur_rot, cur_vel,
                    model->delta_last_pos, model->delta_last_rot, model->delta_last_vel);
                cursor += written;
            }

            if (cursor > 8) {
                sync_batch_buf.resize(cursor);
                wu32(0, STATE_DUMP_MAGIC);
                wu32(4, (uint32_t)(cursor - 8));
                node->rpc("_sync_entities", sync_batch_buf);
            }
        }
        #else
        {
            std::vector<Engine::ISyncable*> snapshot;
            {
                std::lock_guard<std::mutex> lock(sync_models_mutex);
                snapshot = sync_models;
            }
            if (snapshot.empty() || !node || !is_connected()) return;

            int my_id = get_peer_id();
            uint32_t dirty_count = 0;
            sync_batch_buf.resize(12 + (int)snapshot.size() * Engine::ISyncable::SYNC_PACKET_MAX);

            // Client delta batch: [sender u32][VSST magic u32][payload_bytes u32][entries]
            sync_batch_buf.resize(12 + (int)snapshot.size() * Engine::ISyncable::SYNC_PACKET_MAX);
            int cursor = 12;

            auto wu32 = [&](int off, uint32_t v) {
                sync_batch_buf[off]   =  v        & 0xFF;
                sync_batch_buf[off+1] = (v >>  8) & 0xFF;
                sync_batch_buf[off+2] = (v >> 16) & 0xFF;
                sync_batch_buf[off+3] = (v >> 24) & 0xFF;
            };

            for (auto* model : snapshot) {
                if (!model->is_sync_active()) continue;
                if (model->get_sync_authority() != my_id) continue;

                godot::Vector3 cur_pos = model->get_sync_position();
                godot::Vector3 cur_rot = model->get_sync_rotation();

                bool moved = (cur_pos - model->sync_last_pos).length() > 0.001f
                          || (cur_rot - model->sync_last_rot).length() > 0.001f;

                if (!moved) {
                    if (model->sync_sleeping) continue;
                    model->sync_sleeping = true;
                } else {
                    model->sync_sleeping = false;
                }

                model->sync_accum += static_cast<float>(delta);
                if (model->sync_accum < sync_interval && !model->sync_sleeping) continue;

                godot::Vector3 cur_vel = (cur_pos - model->sync_last_pos) / sync_interval;
                model->sync_accum    = 0.0f;
                model->sync_last_pos = cur_pos;
                model->sync_last_rot = cur_rot;
                model->sync_last_vel = cur_vel;

                int written = Engine::ISyncable::encode_delta_public(
                    sync_batch_buf, cursor,
                    model->get_net_id(), cur_pos, cur_rot, cur_vel,
                    model->delta_last_pos, model->delta_last_rot, model->delta_last_vel);
                cursor += written;
            }

            if (cursor > 12) {
                sync_batch_buf.resize(cursor);
                wu32(0, (uint32_t)my_id);
                wu32(4, STATE_DUMP_MAGIC);
                wu32(8, (uint32_t)(cursor - 12));
                node->rpc_id(1, "_sync_client", sync_batch_buf);
            }
        }
        #endif

    }
}
