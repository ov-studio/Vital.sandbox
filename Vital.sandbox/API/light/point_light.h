/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: light: point_light.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Point Light APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/point_light.h>
#include <Vital.sandbox/API/light/light_3d.h>


//////////////////////////////
// Vital: API: Point_Light //
//////////////////////////////

namespace Vital::Sandbox::API {
    struct Point_Light : vm_module {
        inline static const std::vector<std::string> base_scope = {"light", "point"};
        using base_class = Vital::Engine::Point_Light;

        inline static const std::vector<std::pair<std::string, base_class::BakeMode>> bake_mode_registry = {
            { "DISABLED", base_class::BAKE_DISABLED },
            { "STATIC",   base_class::BAKE_STATIC   },
            { "DYNAMIC",  base_class::BAKE_DYNAMIC  }
        };

        inline static const std::vector<std::pair<std::string, base_class::ShadowMode>> shadow_mode_registry = {
            { "DUAL_PARABOLOID", base_class::SHADOW_DUAL_PARABOLOID },
            { "CUBE",            base_class::SHADOW_CUBE            }
        };

        struct Instance : vm_instance<Instance> {
            using Owner = Point_Light;
            base_class* light = nullptr;

            auto get_node() {
                return light;
            }

            bool is_alive() const {
                return light ? true : false;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> light) {
                    instance -> light -> destroy();
                    instance -> light = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;



        static void bind(Machine* vm) {
            vm_module::register_type<Point_Light>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                auto instance = Instance::init(vm);
                instance -> light = base_class::create();
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Light_3D::methods<Instance, Light_3D::Type::Point>(vm);

            vm_module::bind_method<Instance>(vm, "get_shadow_mode", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_shadow_mode());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shadow_mode", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, shadow_mode_registry);

                auto mode = static_cast<base_class::ShadowMode>(vm -> get_int(2));
                self -> light -> set_shadow_mode(mode);
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            API::Light_3D::inject<Instance>(vm);
            vm -> scope_set_enum(base_scope, "shadow_mode", shadow_mode_registry);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Point_Light : vm_module {};
}
#endif
