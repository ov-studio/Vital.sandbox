/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: light: directional_light.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Directional Light APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/directional_light.h>
#include <Vital.sandbox/API/light/light_3d.h>


//////////////////////////////////
// Vital: API: Directional_Light //
//////////////////////////////////

namespace Vital::Sandbox::API {
    struct Directional_Light : vm_module {
        inline static const std::vector<std::string> base_scope = {"light", "directional_light"};
        using base_class = Vital::Engine::Directional_Light;

        inline static const std::vector<std::pair<std::string, base_class::BakeMode>> bake_mode_registry = {
            { "DISABLED", base_class::BAKE_DISABLED },
            { "STATIC",   base_class::BAKE_STATIC   },
            { "DYNAMIC",  base_class::BAKE_DYNAMIC  }
        };

        inline static const std::vector<std::pair<std::string, base_class::ShadowMode>> shadow_mode_registry = {
            { "ORTHOGONAL",        base_class::SHADOW_ORTHOGONAL        },
            { "PARALLEL_2_SPLITS", base_class::SHADOW_PARALLEL_2_SPLITS },
            { "PARALLEL_4_SPLITS", base_class::SHADOW_PARALLEL_4_SPLITS }
        };

        inline static const std::vector<std::pair<std::string, base_class::SkyMode>> sky_mode_registry = {
            { "LIGHT_AND_SKY", base_class::SKY_MODE_LIGHT_AND_SKY },
            { "LIGHT_ONLY",    base_class::SKY_MODE_LIGHT_ONLY    },
            { "SKY_ONLY",      base_class::SKY_MODE_SKY_ONLY      }
        };

        struct Instance : vm_instance<Instance> {
            using Owner = Directional_Light;
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
            vm_module::register_type<Directional_Light>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                auto instance = Instance::init(vm);
                instance -> light = base_class::create();
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Light_3D::methods<Instance>(vm);

            vm_module::bind_method<Instance>(vm, "is_blend_splits_enabled", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> is_blend_splits_enabled());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_shadow_mode", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_shadow_mode());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_shadow_split_1_offset", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_SHADOW_SPLIT_1_OFFSET));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_shadow_split_2_offset", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_SHADOW_SPLIT_2_OFFSET));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_shadow_split_3_offset", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_SHADOW_SPLIT_3_OFFSET));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_shadow_fade_start", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_SHADOW_FADE_START));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_shadow_max_distance", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_SHADOW_MAX_DISTANCE));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_sky_mode", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_sky_mode());
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

            vm_module::bind_method<Instance>(vm, "set_blend_splits_enabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> light -> set_blend_splits(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shadow_split_1_offset", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(offset)", true)
                    .require(2, &Machine::is_number);

                auto offset = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_SHADOW_SPLIT_1_OFFSET, offset);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shadow_split_2_offset", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(offset)", true)
                    .require(2, &Machine::is_number);

                auto offset = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_SHADOW_SPLIT_2_OFFSET, offset);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shadow_split_3_offset", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(offset)", true)
                    .require(2, &Machine::is_number);

                auto offset = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_SHADOW_SPLIT_3_OFFSET, offset);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shadow_fade_start", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(fade)", true)
                    .require(2, &Machine::is_number);

                auto fade = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_SHADOW_FADE_START, fade);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shadow_max_distance", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(distance)", true)
                    .require(2, &Machine::is_number);

                auto distance = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_SHADOW_MAX_DISTANCE, distance);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_sky_mode", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, sky_mode_registry);

                auto mode = static_cast<base_class::SkyMode>(vm -> get_int(2));
                self -> light -> set_sky_mode(mode);
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            API::Light_3D::inject<Instance>(vm);
            vm -> scope_set_enum(base_scope, "shadow_mode", shadow_mode_registry);
            vm -> scope_set_enum(base_scope, "sky_mode", sky_mode_registry);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Directional_Light : vm_module {};
}
#endif
