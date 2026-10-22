/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: base: syncable.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Syncable APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/syncable.h>
#include <Vital.sandbox/Manager/public/network.h>


///////////////////////////
// Vital: API: Syncable //
///////////////////////////

namespace Vital::Sandbox::API {
    struct Syncable {
        // Authority Guard //
        // A replicated entity is owned by the server or by exactly one client. The server is always trusted,
        // a client may only modify a replicated entity it owns, and client-local entities (net_id == 0) are unrestricted
        //
        // Returns the syncable that governs authority over `node`: the node itself if it is one, or its parent for
        // children marked ISyncChild (shapes, wheels, ...). Any other node is unrestricted (nullptr)
        static Vital::Engine::ISyncable* resolve_syncable(godot::Node* node) {
            if (!node) return nullptr;
            if (auto sync = dynamic_cast<Vital::Engine::ISyncable*>(node)) return sync;
            if (dynamic_cast<Vital::Engine::ISyncChild*>(node)) return dynamic_cast<Vital::Engine::ISyncable*>(node -> get_parent());
            return nullptr;
        }

        #if !defined(VSDK_Client)
        // Validates the peer passed to set_syncer(): 0 is the server, any other id must be a connected peer (throws a VM error otherwise)
        // Returns the engine authority id, where the server is stored as 1 (Godot's server peer id)
        static int resolve_syncer(int peer_id) {
            if (peer_id == 0) return 1;
            Vital::Manager::Network::get_singleton() -> assert_peer(peer_id);
            return peer_id;
        }
        #endif

        // Returns whether the caller may modify `node`: always on the server, on the client only for entities it owns or client-local ones
        static bool is_authorized(godot::Node* node) {
            #if !defined(VSDK_Client)
            return true;
            #else
            auto sync = resolve_syncable(node);
            if (!sync || !sync -> is_replicated()) return true;
            auto net = Vital::Manager::Network::get_singleton();
            int authority = sync -> get_sync_authority();
            return net && (authority > 1) && (net -> get_peer_id() == authority);
            #endif
        }

        // Throws (surfaced to Lua as a VM error) when the caller may not modify `node`.
        static void require_authority(godot::Node* node) {
            #if defined(VSDK_Client)
            if (is_authorized(node)) return;
            auto sync = resolve_syncable(node);
            if (sync && sync -> get_sync_authority() > 1) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "cannot modify an entity owned by another client");
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "cannot modify a server-owned entity from the client");
            #endif
        }

        // Drop-in replacement for `vm_module::bind_method` for methods that mutate replicated state.
        template<typename Instance>
        static void bind_method(Machine* vm, const std::string& name, std::function<int(Machine*, std::shared_ptr<Instance>, const std::string&)> exec) {
            vm_module::bind_method<Instance>(vm, name, [exec = std::move(exec)](Machine* vm, std::shared_ptr<Instance> self, const std::string& id) -> int {
                require_authority(self -> get_node());
                return exec(vm, self, id);
            });
        }

        // Binds the methods shared by every syncable instance
        template<typename Instance>
        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "get_net_id", [](auto vm, auto self, auto& id) -> int {
                auto node = self -> get_node();
                if (node -> is_replicated()) vm -> push_value((int)node -> get_net_id());
                else vm -> push_value(false);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_syncer", [](auto vm, auto self, auto& id) -> int {
                auto authority = self -> get_node() -> get_sync_authority();
                vm -> push_value(authority == 1 ? 0 : authority);
                return 1;
            });
            
            #if !defined(VSDK_Client)
            vm_module::bind_method<Instance>(vm, "set_syncer", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(peer_id)", true)
                    .require(2, &Machine::is_number);

                auto peer_id = resolve_syncer((int)vm -> get_int(2));
                self -> get_node() -> set_syncer(peer_id);
                vm -> push_value(true);
                return 1;
            });
            #endif
        }
    };
}
