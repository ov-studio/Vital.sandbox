/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: audio_3d.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Audio 3D APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/audio_3d.h>
#include <Vital.sandbox/API/core/node_3d.h>
#include <Vital.sandbox/API/core/audio_effect.h>
#include <Vital.sandbox/API/utility/file.h>


///////////////////////////
// Vital: API: Audio_3D //
///////////////////////////

namespace Vital::Sandbox::API {
    struct Audio_3D : vm_module {
        inline static const std::vector<std::string> base_scope = {"core", "audio_3d"};
        using base_class = Vital::Engine::Audio_3D;

        inline static const std::vector<std::pair<std::string, godot::AudioStreamPlayer3D::AttenuationModel>> attenuation_model_registry = {
            { "INVERSE_DISTANCE",        godot::AudioStreamPlayer3D::ATTENUATION_INVERSE_DISTANCE        },
            { "INVERSE_SQUARE_DISTANCE", godot::AudioStreamPlayer3D::ATTENUATION_INVERSE_SQUARE_DISTANCE },
            { "LOGARITHMIC",             godot::AudioStreamPlayer3D::ATTENUATION_LOGARITHMIC             },
            { "DISABLED",                godot::AudioStreamPlayer3D::ATTENUATION_DISABLED                }
        };

        inline static const std::vector<std::pair<std::string, godot::AudioStreamPlayer3D::DopplerTracking>> doppler_tracking_registry = {
            { "DISABLED",     godot::AudioStreamPlayer3D::DOPPLER_TRACKING_DISABLED     },
            { "IDLE_STEP",    godot::AudioStreamPlayer3D::DOPPLER_TRACKING_IDLE_STEP    },
            { "PHYSICS_STEP", godot::AudioStreamPlayer3D::DOPPLER_TRACKING_PHYSICS_STEP }
        };

        struct Instance : vm_instance<Instance> {
            using Owner = Audio_3D;
            base_class* audio = nullptr;

            bool is_alive() const {
                return audio ? true : false;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> audio) {
                    instance -> audio -> destroy();
                    instance -> audio = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static void bind(Machine* vm) {
            vm_module::register_type<Audio_3D>(vm);
            API::Node_3D::bind<Instance>(vm);
            API::Audio_Effect::bind<Instance>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path)")
                    .require(1, &Machine::is_string);

                auto path = vm -> get_string(1);
                auto base = API::File::assert_file(vm, path);
                auto instance = Instance::init(vm);
                instance -> audio = base_class::create(base, path);
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Node_3D::methods<Instance, Node_3D::Type::Audio>(vm);
            API::Audio_Effect::methods<Instance>(vm, base_scope);

            // Playback //
            vm_module::bind_method<Instance>(vm, "play", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(position = 0.0)", true)
                    .optional(2, &Machine::is_number);

                self -> audio -> get_player() -> play(vm -> is_number(2) ? vm -> get_float(2) : 0.0f);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "stop", [](auto vm, auto self, auto& id) -> int {
                self -> audio -> get_player() -> stop();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "seek", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(position)", true)
                    .require(2, &Machine::is_number);

                self -> audio -> get_player() -> seek(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_playing", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> is_playing());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_playback_position", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_playback_position());
                return 1;
            });

            // Volume //
            vm_module::bind_method<Instance>(vm, "set_volume", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(volume)", true)
                    .require(2, &Machine::is_number);

                self -> audio -> get_player() -> set_volume_linear(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_volume", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_volume_linear());
                return 1;
            });

            // Pitch //
            vm_module::bind_method<Instance>(vm, "set_pitch_scale", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(pitch_scale)", true)
                    .require(2, &Machine::is_number);

                self -> audio -> get_player() -> set_pitch_scale(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_pitch_scale", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_pitch_scale());
                return 1;
            });

            // Autoplay / Pause //
            vm_module::bind_method<Instance>(vm, "set_autoplay", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(enable)", true)
                    .require(2, &Machine::is_bool);

                self -> audio -> get_player() -> set_autoplay(vm -> get_bool(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_autoplayed", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> is_autoplay_enabled());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_paused", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(pause)", true)
                    .require(2, &Machine::is_bool);

                self -> audio -> get_player() -> set_stream_paused(vm -> get_bool(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_paused", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_stream_paused());
                return 1;
            });

            // Panning //
            vm_module::bind_method<Instance>(vm, "set_panning_strength", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(strength)", true)
                    .require(2, &Machine::is_number);

                self -> audio -> get_player() -> set_panning_strength(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_panning_strength", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_panning_strength());
                return 1;
            });

            // Positional (3D-specific) //
            vm_module::bind_method<Instance>(vm, "set_unit_size", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(unit_size)", true)
                    .require(2, &Machine::is_number);

                self -> audio -> get_player() -> set_unit_size(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_unit_size", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_unit_size());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_max_db", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(max_db)", true)
                    .require(2, &Machine::is_number);

                self -> audio -> get_player() -> set_max_db(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_max_db", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_max_db());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_max_distance", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(meters)", true)
                    .require(2, &Machine::is_number);

                self -> audio -> get_player() -> set_max_distance(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_max_distance", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_max_distance());
                return 1;
            });

            // Attenuation / Doppler //
            vm_module::bind_method<Instance>(vm, "set_attenuation_model", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(model)", true)
                    .require_enum(2, attenuation_model_registry);

                self -> audio -> get_player() -> set_attenuation_model(static_cast<godot::AudioStreamPlayer3D::AttenuationModel>(vm -> get_int(2)));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_attenuation_model", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(static_cast<int>(self -> audio -> get_player() -> get_attenuation_model()));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_doppler_tracking", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, doppler_tracking_registry);

                self -> audio -> get_player() -> set_doppler_tracking(static_cast<godot::AudioStreamPlayer3D::DopplerTracking>(vm -> get_int(2)));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_doppler_tracking", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(static_cast<int>(self -> audio -> get_player() -> get_doppler_tracking()));
                return 1;
            });

            // Emission Angle //
            vm_module::bind_method<Instance>(vm, "set_emission_angle_enabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(enabled)", true)
                    .require(2, &Machine::is_bool);

                self -> audio -> get_player() -> set_emission_angle_enabled(vm -> get_bool(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_emission_angle_enabled", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> is_emission_angle_enabled());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_emission_angle", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(degrees)", true)
                    .require(2, &Machine::is_number);

                self -> audio -> get_player() -> set_emission_angle(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_emission_angle", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_emission_angle());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_emission_angle_filter_attenuation_db", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(db)", true)
                    .require(2, &Machine::is_number);

                self -> audio -> get_player() -> set_emission_angle_filter_attenuation_db(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_emission_angle_filter_attenuation_db", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_emission_angle_filter_attenuation_db());
                return 1;
            });

            // Attenuation Filter //
            vm_module::bind_method<Instance>(vm, "set_attenuation_filter_cutoff_hz", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(hz)", true)
                    .require(2, &Machine::is_number);

                self -> audio -> get_player() -> set_attenuation_filter_cutoff_hz(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_attenuation_filter_cutoff_hz", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_attenuation_filter_cutoff_hz());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_attenuation_filter_db", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(db)", true)
                    .require(2, &Machine::is_number);

                self -> audio -> get_player() -> set_attenuation_filter_db(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_attenuation_filter_db", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_attenuation_filter_db());
                return 1;
            });
        }

        static void inject(Machine* vm) {
            API::Node_3D::inject<Instance>(vm);
            API::Audio_Effect::inject<Instance>(vm, base_scope);
            vm -> scope_set_enum(base_scope, "attenuation_model", attenuation_model_registry);
            vm -> scope_set_enum(base_scope, "doppler_tracking", doppler_tracking_registry);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Audio_3D : vm_module {};
}
#endif