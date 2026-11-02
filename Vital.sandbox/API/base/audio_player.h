/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: base: audio_player.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 01/11/2026
     Desc: Audio Player APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>


/////////////////////////////
// Vital: API: Audio_Player //
/////////////////////////////

namespace Vital::Sandbox::API {
    struct Audio_Player {
        template<typename Instance>
        static void wire(const std::shared_ptr<Instance>& instance) {
            std::weak_ptr<Instance> weak = instance;
            auto signal = [weak](const char* name) {
                return [weak, name]() {
                    auto self = weak.lock();
                    if (!self || !Instance::find_unlocked(self) || !self -> is_alive()) return;
                    Manager::Sandbox::get_singleton() -> signal(name, Tool::StackValue(self));
                };
            };
            instance -> audio -> set_start_handler(signal("audio:start"));
            instance -> audio -> set_finish_handler(signal("audio:finish"));
        }

        template<typename Instance>
        static void methods(Machine* vm) {
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

            vm_module::bind_method<Instance>(vm, "get_max_distance", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_max_distance());
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

            vm_module::bind_method<Instance>(vm, "set_paused", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> audio -> get_player() -> set_stream_paused(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_looped", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                vm -> push_value(self -> audio -> set_looped(state));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_max_distance", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(value)", true)
                    .require(2, &Machine::is_number);

                auto value = vm -> get_float(2);
                self -> audio -> get_player() -> set_max_distance(value);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_pitch_scale", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(value)", true)
                    .require(2, &Machine::is_number);

                auto value = vm -> get_float(2);
                self -> audio -> get_player() -> set_pitch_scale(value);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_panning_strength", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(value)", true)
                    .require(2, &Machine::is_number);

                auto value = vm -> get_float(2);
                self -> audio -> get_player() -> set_panning_strength(value);
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
    };
}
#endif
