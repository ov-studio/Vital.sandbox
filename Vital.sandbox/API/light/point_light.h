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
#include <Vital.sandbox/API/core/node_3d.h>
#include <Vital.sandbox/API/light/light_3d.h>


//////////////////////////////
// Vital: API: Point_Light //
//////////////////////////////

namespace Vital::Sandbox::API {
    struct Point_Light : vm_module {
        inline static const std::vector<std::string> base_scope = {"light", "point_light"};
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

        static std::shared_ptr<Instance> find_by_ptr(base_class* ptr) {
            if (!ptr) return nullptr;
            std::lock_guard<std::mutex> lock(registry.mutex);
            for (auto& [id, instance] : registry.buffer) {
                if (Instance::find_unlocked(instance) && (instance -> light == ptr)) return instance;
            }
            return nullptr;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Point_Light>(vm);
            API::Node_3D::bind<Instance>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                auto instance = Instance::init(vm);
                instance -> light = base_class::create();
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Node_3D::methods<Instance>(vm);
            API::Light_3D::methods<Instance>(vm);

            // Getters: Point (Omni)-only //
            vm_module::bind_method<Instance>(vm, "get_size", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_SIZE));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_range", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_RANGE));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_attenuation", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_ATTENUATION));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_shadow_mode", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_shadow_mode());
                return 1;
            });

            // Setters: Point (Omni)-only //
            vm_module::bind_method<Instance>(vm, "set_size", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(size)", true)
                    .require(2, &Machine::is_number);

                auto size = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_SIZE, size);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_range", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(range)", true)
                    .require(2, &Machine::is_number);

                auto range = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_RANGE, range);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_attenuation", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(attenuation)", true)
                    .require(2, &Machine::is_number);

                auto attenuation = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_ATTENUATION, attenuation);
                vm -> push_value(true);
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
            API::Node_3D::inject<Instance>(vm);
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
