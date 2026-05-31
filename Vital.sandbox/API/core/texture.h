/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: texture.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Texture APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/texture.h>


//////////////////////////
// Vital: API: Texture //
//////////////////////////

namespace Vital::Sandbox::API {
    struct Texture : vm_module {
        inline static const std::string base_name = "texture";
        using base_class = Vital::Engine::Texture;

        struct Instance : vm_instance<Instance> {
            using Owner = Texture;
            base_class* texture = nullptr;
            bool is_alive() const { return texture ? true : false; }
        };
        inline static vm_registry<Instance> registry;

        static void clean_instance(std::shared_ptr<Instance> instance) {
            if (!Instance::erase(instance)) return;
            if (instance -> texture) {
                instance -> texture -> destroy();
                instance -> texture = nullptr;
            }
            Instance::release(instance);
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Texture>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path)")
                    .require(1, &Machine::is_string);

                auto path = vm -> get_string(1);
                auto instance = Instance::make(vm);
                instance -> texture = base_class::create_texture_2d(path);
                return 1;
            });
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Texture : vm_module {};
}
#endif