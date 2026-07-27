/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: audio_2d.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     Desc: Audio 2D APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/audio_2d.h>
#include <Vital.sandbox/API/utility/file.h>


///////////////////////////
// Vital: API: Audio_2D //
///////////////////////////

namespace Vital::Sandbox::API {
    struct Audio2D : vm_module {
        inline static const std::vector<std::string> base_scope = {"core", "audio_2d"};
        using base_class = Vital::Engine::Audio2D;

        inline static const std::vector<std::pair<std::string, godot::AudioServer::PlaybackType>> playback_type_registry = {
            { "DEFAULT", godot::AudioServer::PLAYBACK_TYPE_DEFAULT },
            { "STREAM",  godot::AudioServer::PLAYBACK_TYPE_STREAM  },
            { "SAMPLE",  godot::AudioServer::PLAYBACK_TYPE_SAMPLE  }
        };

        struct Instance : vm_instance<Instance> {
            using Owner = Audio2D;
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
            vm_module::register_type<Audio2D>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path, volume_db = 0.0, pitch_scale = 1.0, bus = \"Master\", autoplay = false)")
                    .require(1, &Machine::is_string)
                    .optional(2, &Machine::is_number)
                    .optional(3, &Machine::is_number)
                    .optional(4, &Machine::is_string)
                    .optional(5, &Machine::is_bool);

                auto path = vm -> get_string(1);
                auto base = API::File::assert_file(vm, path);
                auto volume_db = vm -> is_number(2) ? vm -> get_float(2) : 0.0f;
                auto pitch_scale = vm -> is_number(3) ? vm -> get_float(3) : 1.0f;
                auto bus = vm -> is_string(4) ? Tool::to_godot_string(vm -> get_string(4)) : godot::StringName("Master");
                auto autoplay = vm -> is_bool(5) ? vm -> get_bool(5) : false;

                auto instance = Instance::init(vm);
                instance -> audio = base_class::create(base, path);
                instance -> audio -> get_player() -> set_volume_db(volume_db);
                instance -> audio -> get_player() -> set_pitch_scale(pitch_scale);
                instance -> audio -> get_player() -> set_bus(bus);
                instance -> audio -> get_player() -> set_autoplay(autoplay);
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "play", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(from_position = 0.0)", true)
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

            vm_module::bind_method<Instance>(vm, "set_volume_db", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(volume_db)", true)
                    .require(2, &Machine::is_number);

                self -> audio -> get_player() -> set_volume_db(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_volume_db", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_volume_db());
                return 1;
            });

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

            vm_module::bind_method<Instance>(vm, "set_bus", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(bus)", true)
                    .require(2, &Machine::is_string);

                self -> audio -> get_player() -> set_bus(Tool::to_godot_string(vm -> get_string(2)));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_bus", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(Tool::to_std_string(self -> audio -> get_player() -> get_bus()));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_autoplay", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(enable)", true)
                    .require(2, &Machine::is_bool);

                self -> audio -> get_player() -> set_autoplay(vm -> get_bool(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_autoplay_enabled", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> is_autoplay_enabled());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_stream_paused", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(pause)", true)
                    .require(2, &Machine::is_bool);

                self -> audio -> get_player() -> set_stream_paused(vm -> get_bool(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_stream_paused", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_stream_paused());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_max_distance", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(pixels)", true)
                    .require(2, &Machine::is_number);

                self -> audio -> get_player() -> set_max_distance(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_max_distance", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_max_distance());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_attenuation", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(curve)", true)
                    .require(2, &Machine::is_number);

                self -> audio -> get_player() -> set_attenuation(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_attenuation", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_attenuation());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_area_mask", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mask)", true)
                    .require(2, &Machine::is_number);

                self -> audio -> get_player() -> set_area_mask(static_cast<uint32_t>(vm -> get_double(2)));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_area_mask", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(static_cast<int>(self -> audio -> get_player() -> get_area_mask()));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_max_polyphony", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(voices)", true)
                    .require(2, &Machine::is_number);

                self -> audio -> get_player() -> set_max_polyphony(vm -> get_int(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_max_polyphony", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_max_polyphony());
                return 1;
            });

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

            vm_module::bind_method<Instance>(vm, "set_playback_type", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(type)", true)
                    .require_enum(2, playback_type_registry);

                self -> audio -> get_player() -> set_playback_type(static_cast<godot::AudioServer::PlaybackType>(vm -> get_int(2)));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_playback_type", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(static_cast<int>(self -> audio -> get_player() -> get_playback_type()));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "has_stream_playback", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> has_stream_playback());
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "playback_type", playback_type_registry);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Audio2D : vm_module {};
}
#endif