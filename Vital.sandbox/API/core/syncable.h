/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: syncable.h
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


//////////////////////////////
// Vital: API: Syncable //
//////////////////////////////

// TODO: Improve
// Shared surface for every API type whose underlying engine object derives
// from Engine::ISyncable — Model and every Physics_Body<Base> specialization
// (Rigid/Static/Character/Animatable/Vehicle). get_node() on those types
// already returns a pointer that publicly inherits ISyncable, so get_net_id()
// and is_replicated() are plain accessible members — no cast needed. Bound
// once here so every syncable entity gets identical get_net_id() semantics
// instead of each API type re-implementing the same "0 == not replicated"
// check (Physics_Body used to do this inline; Model never exposed net_id to
// Lua at all).
//
// Deliberately NOT gated behind #if !defined(VSDK_Client): a client-side
// remote mirror of a server entity carries the real net_id too (assigned via
// Network::_spawn_entity), and Manager::Network::find_syncable() — the O(1)
// net_id lookup this pairs with (see API::Engine::get_entity_by_net_id) — is
// itself documented as available on both client and server builds. Reading
// your own net_id is harmless; only mutating authority (set_syncer) needs to
// stay server-only, and that binding is untouched.
namespace Vital::Sandbox::API {
    struct Syncable {
        template<typename Instance>
        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "get_net_id", [](auto vm, auto self, auto& id) -> int {
                auto* node = self -> get_node();
                // net_id == 0 means this entity was never registered with the
                // network layer (client-local / not (yet) replicated) — surface
                // that as `false` in Lua rather than a meaningless 0 id.
                if (node && node -> is_replicated()) vm -> push_value((int)node -> get_net_id());
                else vm -> push_value(false);
                return 1;
            });
        }
    };
}
