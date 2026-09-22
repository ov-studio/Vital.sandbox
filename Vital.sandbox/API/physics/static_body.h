/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: physics: static_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Static Body APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/static_body.h>
#include <Vital.sandbox/API/base/physics_body.h>


//////////////////////////////
// Vital: API: Static_Body //
//////////////////////////////

namespace Vital::Sandbox::API {
    struct Static_Body : vm_module {
        inline static const std::vector<std::string> base_scope = {"physics", "static"};
        inline static constexpr bool has_remote = true;
        inline static constexpr bool has_streaming = true;
        using base_class = Vital::Engine::Static_Body;

        struct Instance : Physics_Body_Instance<Instance, base_class> {
            using Owner = Static_Body;
        };
        inline static vm_registry<Instance> registry;

        static void bind(Machine* vm) {
            vm_module::register_type<Static_Body>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                int authority_peer = 0;
                #if !defined(VSDK_Client)
                if (vm -> is_number(1)) authority_peer = vm -> get_int(1);
                #endif
                auto instance = Instance::init(vm);
                instance -> body = base_class::create(authority_peer);
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Physics_Body::methods<Instance, Physics_Body::Type::Static>(vm);
        }

        static void inject(Machine* vm) {
            API::Physics_Body::inject<Instance>(vm);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
