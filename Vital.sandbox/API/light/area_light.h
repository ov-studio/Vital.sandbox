/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: light: area_light.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Area Light APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/area_light.h>
#include <Vital.sandbox/API/core/node_3d.h>


/////////////////////////////
// Vital: API: Area Light //
/////////////////////////////

namespace Vital::Sandbox::API {
    struct Area_Light : vm_module {
        inline static const std::vector<std::string> base_scope = {"light", "area_light"};
        using base_class = Vital::Engine::Area_Light;

        inline static const std::vector<std::pair<std::string, base_class::BakeMode>> bake_mode_registry = {
            { "DISABLED", base_class::BAKE_DISABLED },
            { "STATIC",   base_class::BAKE_STATIC   },
            { "DYNAMIC",  base_class::BAKE_DYNAMIC  }
        };

        struct Instance : vm_instance<Instance> {
            using Owner = Area_Light;
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
            vm_module::register_type<Area_Light>(vm);
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

            // Checkers //
            vm_module::bind_method<Instance>(vm, "is_shadow_enabled", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> has_shadow());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_negative", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> is_negative());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_shadow_reverse_cull_face_enabled", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_shadow_reverse_cull_face());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_distance_fade_enabled", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> is_distance_fade_enabled());
                return 1;
            });

            // Getters: Light3D (common) //
            vm_module::bind_method<Instance>(vm, "get_color", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_color());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_energy", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_ENERGY));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_indirect_energy", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_INDIRECT_ENERGY));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_volumetric_fog_energy", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_VOLUMETRIC_FOG_ENERGY));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_specular", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_SPECULAR));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_intensity", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_INTENSITY));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_bake_mode", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_bake_mode());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_cull_mask", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_cull_mask());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_temperature", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_temperature());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_correlated_color", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_correlated_color());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_shadow_bias", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_SHADOW_BIAS));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_shadow_normal_bias", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_SHADOW_NORMAL_BIAS));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_shadow_opacity", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_SHADOW_OPACITY));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_shadow_blur", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_SHADOW_BLUR));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_transmittance_bias", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_param(base_class::PARAM_TRANSMITTANCE_BIAS));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_distance_fade_begin", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_distance_fade_begin());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_distance_fade_shadow", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_distance_fade_shadow());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_distance_fade_length", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_distance_fade_length());
                return 1;
            });

            // Getters: Area-only (DRAFT — verify against local headers) //
            vm_module::bind_method<Instance>(vm, "get_area", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> light -> get_area_size());
                return 1;
            });

            // Setters: Light3D (common) //
            vm_module::bind_method<Instance>(vm, "set_color", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(color)", true)
                    .require(2, &Machine::is_color);

                auto color = vm -> get_color(2);
                self -> light -> set_color(color);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_energy", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(energy)", true)
                    .require(2, &Machine::is_number);

                auto energy = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_ENERGY, energy);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_indirect_energy", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(energy)", true)
                    .require(2, &Machine::is_number);

                auto energy = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_INDIRECT_ENERGY, energy);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_volumetric_fog_energy", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(energy)", true)
                    .require(2, &Machine::is_number);

                auto energy = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_VOLUMETRIC_FOG_ENERGY, energy);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_specular", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(specular)", true)
                    .require(2, &Machine::is_number);

                auto specular = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_SPECULAR, specular);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_intensity", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(intensity)", true)
                    .require(2, &Machine::is_number);

                auto intensity = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_INTENSITY, intensity);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_negative", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> light -> set_negative(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_bake_mode", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, bake_mode_registry);

                auto mode = static_cast<base_class::BakeMode>(vm -> get_int(2));
                self -> light -> set_bake_mode(mode);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_cull_mask", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mask)", true)
                    .require(2, &Machine::is_number);

                auto mask = vm -> get_int(2);
                self -> light -> set_cull_mask(mask);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_temperature", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(kelvin)", true)
                    .require(2, &Machine::is_number);

                auto kelvin = vm -> get_float(2);
                self -> light -> set_temperature(kelvin);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shadow_enabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> light -> set_shadow(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shadow_bias", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(bias)", true)
                    .require(2, &Machine::is_number);

                auto bias = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_SHADOW_BIAS, bias);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shadow_normal_bias", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(bias)", true)
                    .require(2, &Machine::is_number);

                auto bias = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_SHADOW_NORMAL_BIAS, bias);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shadow_opacity", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(opacity)", true)
                    .require(2, &Machine::is_number);

                auto opacity = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_SHADOW_OPACITY, opacity);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shadow_blur", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(blur)", true)
                    .require(2, &Machine::is_number);

                auto blur = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_SHADOW_BLUR, blur);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_shadow_reverse_cull_face_enabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> light -> set_shadow_reverse_cull_face(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_transmittance_bias", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(bias)", true)
                    .require(2, &Machine::is_number);

                auto bias = vm -> get_float(2);
                self -> light -> set_param(base_class::PARAM_TRANSMITTANCE_BIAS, bias);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_distance_fade_enabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> light -> set_enable_distance_fade(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_distance_fade_begin", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(distance)", true)
                    .require(2, &Machine::is_number);

                auto distance = vm -> get_float(2);
                self -> light -> set_distance_fade_begin(distance);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_distance_fade_shadow", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(distance)", true)
                    .require(2, &Machine::is_number);

                auto distance = vm -> get_float(2);
                self -> light -> set_distance_fade_shadow(distance);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_distance_fade_length", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(distance)", true)
                    .require(2, &Machine::is_number);

                auto distance = vm -> get_float(2);
                self -> light -> set_distance_fade_length(distance);
                vm -> push_value(true);
                return 1;
            });

            // Setters: Area-only (DRAFT — verify against local headers) //
            vm_module::bind_method<Instance>(vm, "set_area", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(size)", true)
                    .require(2, &Machine::is_vector2);

                auto size = vm -> get_vector2(2);
                self -> light -> set_area_size(size);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_texture", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(path)", true)
                    .require(2, &Machine::is_string);

                auto path = vm -> get_string(2);
                auto ref = path;
                auto base = API::File::assert_file(vm, path);
                auto texture = Vital::Engine::Texture::get_from_reference(ref);
                if (!texture) texture = Vital::Engine::Texture::create_texture_2d(base, path, true, ref);
                self -> light -> set_area_texture(texture -> get_texture());
                auto texture_reference = fmt::format("{}:texture:{}", vm_module::scope_id(base_scope), (void*) self -> light);
                vm -> push_value(path);
                vm -> set_reference("sandbox", texture_reference, -1);
                vm -> pop(1);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "reset_texture", [](auto vm, auto self, auto& id) -> int {
                self -> light -> set_area_texture(godot::Ref<godot::Texture2D>());
                auto texture_reference = fmt::format("{}:texture:{}", vm_module::scope_id(base_scope), (void*) self -> light);
                vm -> del_reference("sandbox", texture_reference);
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            API::Node_3D::inject<Instance>(vm);
            vm -> scope_set_enum(base_scope, "bake_mode", bake_mode_registry);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Area_Light : vm_module {};
}
#endif