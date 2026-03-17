/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: network.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network System
----------------------------------------------------------------*/


#pragma once
#include <Vital.extension/Engine/public/index.h>
#include <godot_cpp/classes/e_net_multiplayer_peer.hpp>
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/multiplayer_peer.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <unordered_set>
#include <functional>
#include <Vital.extension/Engine/public/console.h>


/////////////////////////////
// Vital: System: Network //
/////////////////////////////

namespace Vital::Engine {

    // Bridge node — registered in GDExtension _register_types
    class NetworkBridge : public godot::Node {
        GDCLASS(NetworkBridge, godot::Node)
        public:
            static void _bind_methods() {
                godot::ClassDB::bind_method(godot::D_METHOD("_on_peer_connected",    "id"), &NetworkBridge::_on_peer_connected);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_peer_disconnected", "id"), &NetworkBridge::_on_peer_disconnected);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_connected_to_server"),     &NetworkBridge::_on_connected_to_server);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_connection_failed"),       &NetworkBridge::_on_connection_failed);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_server_disconnected"),     &NetworkBridge::_on_server_disconnected);
            }

            std::function<void(int)> on_peer_connected;
            std::function<void(int)> on_peer_disconnected;
            std::function<void()>    on_connected_to_server;
            std::function<void()>    on_connection_failed;
            std::function<void()>    on_server_disconnected;

            void _on_peer_connected(int id)    { if (on_peer_connected)      on_peer_connected(id); }
            void _on_peer_disconnected(int id) { if (on_peer_disconnected)   on_peer_disconnected(id); }
            void _on_connected_to_server()     { if (on_connected_to_server) on_connected_to_server(); }
            void _on_connection_failed()       { if (on_connection_failed)   on_connection_failed(); }
            void _on_server_disconnected()     { if (on_server_disconnected) on_server_disconnected(); }
    };


    class Network {
        private:
            inline static Network* singleton = nullptr;
            godot::Ref<godot::ENetMultiplayerPeer> peer;
            NetworkBridge* bridge = nullptr;
            std::unordered_set<int> connected_peers;

            bool        auto_reconnect     = false;
            std::string reconnect_ip;
            int         reconnect_port     = 0;
            int         reconnect_attempts = 0;
            int         reconnect_max      = 5;
            float       reconnect_delay    = 3.0f;
            float       reconnect_timer    = 0.0f;

            static godot::SceneTree* get_scene_tree() {
                return godot::Object::cast_to<godot::SceneTree>(
                    godot::Engine::get_singleton()->get_main_loop()
                );
            }

            static Vital::Tool::Stack make_stack(int32_t id) {
                Vital::Tool::Stack s;
                s.array.push_back(Vital::Tool::StackValue(id));
                return s;
            }

            void create_bridge() {
                if (bridge) return;
                // NetworkBridge only needs to be a godot::Object for Callable binding.
                // It does NOT need to be in the scene tree — memnew is enough.
                bridge = memnew(NetworkBridge);
                bridge->on_peer_connected      = [this](int id) { _on_peer_connected(id); };
                bridge->on_peer_disconnected   = [this](int id) { _on_peer_disconnected(id); };
                bridge->on_connected_to_server = [this]()       { _on_connected_to_server(); };
                bridge->on_connection_failed   = [this]()       { _on_connection_failed(); };
                bridge->on_server_disconnected = [this]()       { _on_server_disconnected(); };
            }

            void destroy_bridge() {
                if (!bridge) return;
                memdelete(bridge);
                bridge = nullptr;
            }

            void wire_server_signals() {
                create_bridge();
                if (!bridge) return;
                auto mp = get_scene_tree()->get_multiplayer();
                if (!mp.is_valid()) return;
                mp->connect("peer_connected",    godot::Callable(bridge, "_on_peer_connected"));
                mp->connect("peer_disconnected", godot::Callable(bridge, "_on_peer_disconnected"));
            }

            void wire_client_signals() {
                create_bridge();
                if (!bridge) return;
                auto mp = get_scene_tree()->get_multiplayer();
                if (!mp.is_valid()) return;
                mp->connect("connected_to_server", godot::Callable(bridge, "_on_connected_to_server"));
                mp->connect("connection_failed",   godot::Callable(bridge, "_on_connection_failed"));
                mp->connect("server_disconnected", godot::Callable(bridge, "_on_server_disconnected"));
            }

            void unwire_signals() {
                auto tree = get_scene_tree();
                if (!tree || !bridge) return;
                auto mp = tree->get_multiplayer();
                if (!mp.is_valid()) return;
                
                auto try_disconnect = [&](const char* signal, const char* method) {
                    godot::Callable cb(bridge, method);
                    if (mp->is_connected(signal, cb))
                        mp->disconnect(signal, cb);
                };
                try_disconnect("peer_connected",      "_on_peer_connected");
                try_disconnect("peer_disconnected",   "_on_peer_disconnected");
                try_disconnect("connected_to_server", "_on_connected_to_server");
                try_disconnect("connection_failed",   "_on_connection_failed");
                try_disconnect("server_disconnected", "_on_server_disconnected");
            }

        public:
            Network()  = default;
            ~Network() = default;

            static Network* get_singleton() {
                if (!singleton) singleton = new Network();
                return singleton;
            }

            static void free_singleton() {
                if (!singleton) return;
                if (is_server()) singleton->close();
                else             singleton->disconnect_from_server();
                singleton->unwire_signals();
                singleton->destroy_bridge();
                if (singleton->peer.is_valid()) {
                    singleton->peer -> close();
                    singleton->peer.unref();
                }
                delete singleton;
                singleton = nullptr;
            }


            //----------------//
            //     State      //
            //----------------//

            bool is_active() const {
                return peer.is_valid() &&
                    peer -> get_connection_status() == godot::MultiplayerPeer::CONNECTION_CONNECTED;
            }

            bool is_connecting() const {
                return peer.is_valid() &&
                    peer -> get_connection_status() == godot::MultiplayerPeer::CONNECTION_CONNECTING;
            }

            static bool is_server() {
                return get_platform() == "server";
            }

            int get_peer_id() const {
                return peer.is_valid() ? peer -> get_unique_id() : 0;
            }

            const std::unordered_set<int>& get_connected_peers() const {
                return connected_peers;
            }

            int get_peer_count() const {
                return static_cast<int>(connected_peers.size());
            }

            void print_status() const {
                Vital::print("sbox", "=== Network Status ===");
                Vital::print("sbox", "Role   : ", is_server() ? "SERVER" : "CLIENT");
                Vital::print("sbox", "Active : ", is_active());
                Vital::print("sbox", "PeerID : ", get_peer_id());
                if (is_server())
                    Vital::print("sbox", "Clients: ", (int)connected_peers.size());
            }


            //----------------//
            //     Server     //
            //----------------//

            bool host(int port, int max_clients = 32) {
                if (!is_server()) {
                    Vital::print("sbox", "Network: host() called on non-server");
                    return false;
                }
                if (is_active()) {
                    Vital::print("sbox", "Network: already hosting");
                    return false;
                }
                peer.instantiate();
                Vital::print("sbox", "ENet peer instantiated: ", peer.is_valid());  // add this
                godot::Error err = peer -> create_server(port, max_clients);
                Vital::print("sbox", "create_server error code: ", (int)err);       // add this

                if (err != godot::OK) {
                    Vital::print("sbox", "Network: failed to host on port ", port);
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

            bool close() {
                if (!is_server() || !peer.is_valid()) return false;
                unwire_signals();
                connected_peers.clear();
                peer -> close();
                peer.unref();
                auto tree = get_scene_tree();
                if (tree) tree->get_multiplayer()->set_multiplayer_peer(nullptr);
                Vital::print("sbox", "Network: server closed");
                Vital::Tool::Event::emit("network:closed", {});
                return true;
            }

            void _on_peer_connected(int id) {
                connected_peers.insert(id);
                Vital::print("sbox", "Network: peer joined -> ", id, "  total: ", (int)connected_peers.size());
                auto args = make_stack(static_cast<int32_t>(id));
                Vital::Tool::Event::emit("network:peer_joined", args);
            }

            void _on_peer_disconnected(int id) {
                connected_peers.erase(id);
                Vital::print("sbox", "Network: peer left -> ", id, "  remaining: ", (int)connected_peers.size());
                auto args = make_stack(static_cast<int32_t>(id));
                Vital::Tool::Event::emit("network:peer_left", args);
            }


            //----------------//
            //     Client     //
            //----------------//

            bool connect_to_server(const std::string& ip, int port, bool enable_reconnect = false) {
                if (is_active() || is_connecting()) {
                    Vital::print("sbox", "Network: already connected/connecting");
                    return false;
                }
                peer.instantiate();
                if (peer -> create_client(godot::String(ip.c_str()), port) != godot::OK) {
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
                Vital::print("sbox", "Network: connecting to ", ip.c_str(), ":", port);
                Vital::Tool::Event::emit("network:connecting", {});
                return true;
            }

            bool disconnect_from_server() {
                if (!peer.is_valid()) return false;
                auto_reconnect = false;
                unwire_signals();
                if (peer.is_valid()) {
                    peer -> close();
                    peer.unref();
                }
                auto tree = get_scene_tree();
                if (tree) tree->get_multiplayer()->set_multiplayer_peer(nullptr);
                Vital::print("sbox", "Network: disconnected");
                Vital::Tool::Event::emit("network:disconnected", {});
                return true;
            }

            void _on_connected_to_server() {
                reconnect_attempts = 0;
                Vital::print("sbox", "Network: connected  peer_id=", get_peer_id());
                Vital::Tool::Event::emit("network:connected", {});
            }

            void _on_connection_failed() {
                Vital::print("sbox", "Network: connection failed");
                unwire_signals();
                if (peer.is_valid()) peer.unref();
                Vital::Tool::Event::emit("network:connection_failed", {});
                if (auto_reconnect) _schedule_reconnect();
            }

            void _on_server_disconnected() {
                Vital::print("sbox", "Network: server dropped connection");
                unwire_signals();
                if (peer.is_valid()) peer.unref();
                Vital::Tool::Event::emit("network:server_disconnected", {});
                if (auto_reconnect) _schedule_reconnect();
            }


            //--------------------//
            //   Auto-Reconnect   //
            //--------------------//

            void set_reconnect_config(int max_attempts, float delay_seconds) {
                reconnect_max   = max_attempts;
                reconnect_delay = delay_seconds;
            }

            void _schedule_reconnect() {
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


            //--------------------//
            //   Send / Receive   //
            //--------------------//

            bool send(const Vital::Tool::Stack& stack, int peerID = 0, bool isLatent = false) {
                if (!peer.is_valid()) return false;
                if (!is_server() && !is_active()) return false;
                std::string bytes = stack.serialize();
                godot::PackedByteArray packet;
                packet.resize(bytes.size());
                memcpy(packet.ptrw(), bytes.data(), bytes.size());
                peer -> set_target_peer(peerID);
                peer -> set_transfer_mode(isLatent
                    ? godot::MultiplayerPeer::TRANSFER_MODE_UNRELIABLE
                    : godot::MultiplayerPeer::TRANSFER_MODE_RELIABLE
                );
                peer -> put_packet(packet);
                return true;
            }

            bool broadcast(const Vital::Tool::Stack& stack, bool isLatent = false) {
                return send(stack, 0, isLatent);
            }

            bool send_to_server(const Vital::Tool::Stack& stack, bool isLatent = false) {
                return send(stack, 1, isLatent);
            }


            //--------//
            //  Poll  //
            //--------//

            void poll(float delta = 0.0f) {
                if (auto_reconnect && !is_active() && !is_connecting()) {
                    if (reconnect_timer > 0.0f) {
                        reconnect_timer -= delta;
                        if (reconnect_timer <= 0.0f)
                            connect_to_server(reconnect_ip, reconnect_port, true);
                    }
                    return;
                }
 
                if (!peer.is_valid()) return;
                peer -> poll();

                while (peer -> get_available_packet_count() > 0) {
                    godot::PackedByteArray raw = peer -> get_packet();
                    int32_t sender = static_cast<int32_t>(peer -> get_packet_peer());
                    Vital::Tool::Stack stack = Vital::Tool::Stack::deserialize(
                        reinterpret_cast<const char*>(raw.ptr()), raw.size()
                    );
                    stack.object["sender_id"] = Vital::Tool::StackValue(sender);
                    Vital::Tool::Event::emit("network:packet", stack);
                }
            }


            //----------------//
            //  Emit outbound //
            //----------------//

            static void emit(Vital::Tool::Stack& arguments, int peerID = 0, bool isLatent = false) {
                get_singleton()->send(arguments, peerID, isLatent);
            }
    };
}