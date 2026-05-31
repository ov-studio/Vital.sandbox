/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: svg.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SVG APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/texture.h>


//////////////////////
// Vital: API: SVG //
//////////////////////

namespace Vital::Sandbox::API {
    struct SVG : vm_module {
        inline static const std::string base_name = "svg";
        using base_class = Vital::Engine::Texture;

        struct Instance : vm_instance<Instance> {
            using Owner = SVG;
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
            vm_module::register_type<SVG>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path)")
                    .require(1, &Machine::is_string);

                auto path = vm -> get_string(1);
                auto instance = Instance::make(vm);
                instance -> texture = base_class::create_svg(path);
                return 1;
            });

            API::bind(vm, {base_name}, "create_from_raw", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(raw)")
                    .require(1, &Machine::is_string);

                auto raw = vm -> get_string(1);
                auto instance = Instance::make(vm);
                instance -> texture = base_class::create_svg_from_raw(raw);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, base_name, "update", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(raw)")
                    .require(2, &Machine::is_string);

                self -> texture -> update_svg_from_raw(vm -> get_string(2));
                vm -> push_value(true);
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
    struct SVG : vm_module {};
}
#endif