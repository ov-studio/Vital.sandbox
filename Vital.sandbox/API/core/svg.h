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
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/texture.h>
#include <Vital.sandbox/API/utility/file.h>


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

            bool is_alive() const { 
                return texture ? true : false;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> texture) {
                    instance -> texture -> destroy();
                    instance -> texture = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static void bind(Machine* vm) {
            vm_module::register_type<SVG>(vm);

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path)")
                    .require(1, &Machine::is_string);

                auto path = vm -> get_string(1);
                auto base = API::File::assert_file(vm, path);
                auto instance = Instance::init(vm);
                instance -> texture = base_class::create_svg(base, path);
                instance -> store(true);
                return 1;
            });

            API::bind(vm, {base_name}, "create_from_raw", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(raw)")
                    .require(1, &Machine::is_string);

                auto raw = vm -> get_string(1);
                auto instance = Instance::init(vm);
                instance -> texture = base_class::create_svg_from_raw(raw);
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "get_size", [](auto vm, auto self, auto& id) -> int {
                auto size = self -> texture -> get_size();
                vm -> push_value(self -> texture -> get_size());
                return 2;
            });

            vm_module::bind_method<Instance>(vm, "update", [](auto vm, auto self, auto& id) -> int {
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