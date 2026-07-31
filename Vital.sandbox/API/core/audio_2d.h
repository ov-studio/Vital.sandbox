/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: audio_2d.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Audio 2D APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/audio_2d.h>
#include <Vital.sandbox/API/core/audio_effect.h>
#include <Vital.sandbox/API/utility/file.h>


///////////////////////////
// Vital: API: Audio_2D //
///////////////////////////

namespace Vital::Sandbox::API {
    struct Audio_2D : vm_module {
        inline static const std::vector<std::string> base_scope = {"core", "audio_2d"};
        using base_class = Vital::Engine::Audio_2D;

        struct Instance : vm_instance<Instance> {
            using Owner = Audio_2D;
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
            vm_module::register_type<Audio_2D>(vm);
            API::Audio_Effect::bind<Instance>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path, autoplay = false)")
                    .require(1, &Machine::is_string)
                    .optional(2, &Machine::is_bool);

                auto path = vm -> get_string(1);
                auto base = API::File::assert_file(vm, path);
                auto autoplay = vm -> is_bool(2) ? vm -> get_bool(2) : false;
                auto instance = Instance::init(vm);
                instance -> audio = base_class::create(base, path, autoplay);
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Audio_Effect::methods<Instance>(vm);

            vm_module::bind_method<Instance>(vm, "is_playing", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> is_playing());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_paused", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_stream_paused());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_looped", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> is_looped());
                return 1;
            });
            
            vm_module::bind_method<Instance>(vm, "is_autoplayed", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> is_autoplay_enabled());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_volume", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_volume_linear());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_max_distance", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_max_distance());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_attenuation", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_attenuation());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_pitch_scale", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_pitch_scale());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_panning_strength", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_panning_strength());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_playback_position", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_playback_position());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_volume", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(value)", true)
                    .require(2, &Machine::is_number);

                auto volume = vm -> get_float(2);
                self -> audio -> get_player() -> set_volume_linear(volume);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_paused", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto pause = vm -> get_bool(2);
                self -> audio -> get_player() -> set_stream_paused(pause);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_looped", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto loop = vm -> get_bool(2);
                vm -> push_value(self -> audio -> set_looped(loop));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_max_distance", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(value)", true)
                    .require(2, &Machine::is_number);

                auto pixels = vm -> get_float(2);
                self -> audio -> get_player() -> set_max_distance(pixels);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_attenuation", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(value)", true)
                    .require(2, &Machine::is_number);

                auto curve = vm -> get_float(2);
                self -> audio -> get_player() -> set_attenuation(curve);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_pitch_scale", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(value)", true)
                    .require(2, &Machine::is_number);

                auto pitch_scale = vm -> get_float(2);
                self -> audio -> get_player() -> set_pitch_scale(pitch_scale);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_panning_strength", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(value)", true)
                    .require(2, &Machine::is_number);

                auto strength = vm -> get_float(2);
                self -> audio -> get_player() -> set_panning_strength(strength);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "play", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(position = 0.0)", true)
                    .optional(2, &Machine::is_number);

                auto position = vm -> is_number(2) ? vm -> get_float(2) : 0.0f;
                self -> audio -> get_player() -> play(position);
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

                auto position = vm -> get_float(2);
                self -> audio -> get_player() -> seek(position);
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            API::Audio_Effect::inject<Instance>(vm);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Audio_2D : vm_module {};
}
#endif
