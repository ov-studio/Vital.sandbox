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
#include <Vital.sandbox/Engine/public/collision_shape.h>
#include <Vital.sandbox/Engine/public/vehicle_wheel.h>
#include <Vital.sandbox/Manager/public/network.h>


///////////////////////////
// Vital: API: Syncable //
///////////////////////////

namespace Vital::Sandbox::API {
    struct Syncable {
        // Authority Guard //
        // Replicated entities are owned either by the server (authority 1) or by a single client.
        // The server is always trusted. A client may only mutate a replicated entity it owns.
        // Client-local entities (net_id == 0) are never restricted.

        // Resolves the syncable that governs authority over `node`.
        // Collision shapes and vehicle wheels have no authority of their own, they follow
        // the body they are attached to. Any other node (e.g. a client-local prop that was
        // parented under a replicated entity) stays unrestricted.
        static Vital::Engine::ISyncable* resolve(godot::Node* node) {
            if (!node) return nullptr;
            if (auto sync = dynamic_cast<Vital::Engine::ISyncable*>(node)) return sync;
            if (godot::Object::cast_to<Vital::Engine::Collision_Shape>(node) || godot::Object::cast_to<Vital::Engine::Vehicle_Wheel>(node)) return dynamic_cast<Vital::Engine::ISyncable*>(node -> get_parent());
            return nullptr;
        }

        static bool is_authorized(godot::Node* node) {
            #if !defined(VSDK_Client)
            return true;
            #else
            auto sync = resolve(node);
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
            auto sync = resolve(node);
            if (sync && sync -> get_sync_authority() > 1) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: cannot modify an entity owned by another client");
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: cannot modify a server-owned entity from the client");
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


        // Methods //
        template<typename Instance>
        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "get_net_id", [](auto vm, auto self, auto& id) -> int {
                auto node = self -> get_node();
                if (node -> is_replicated()) vm -> push_value((int)node -> get_net_id());
                else vm -> push_value(false);
                return 1;
            });
        }
    };
}
