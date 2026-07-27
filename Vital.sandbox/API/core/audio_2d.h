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
#include <godot_cpp/classes/audio_effect_reverb.hpp>
#include <godot_cpp/classes/audio_effect_chorus.hpp>
#include <godot_cpp/classes/audio_effect_delay.hpp>
#include <godot_cpp/classes/audio_effect_distortion.hpp>
#include <godot_cpp/classes/audio_effect_amplify.hpp>
#include <godot_cpp/classes/audio_effect_compressor.hpp>
#include <godot_cpp/classes/audio_effect_limiter.hpp>
#include <godot_cpp/classes/audio_effect_hard_limiter.hpp>
#include <godot_cpp/classes/audio_effect_panner.hpp>
#include <godot_cpp/classes/audio_effect_phaser.hpp>
#include <godot_cpp/classes/audio_effect_pitch_shift.hpp>
#include <godot_cpp/classes/audio_effect_stereo_enhance.hpp>
#include <godot_cpp/classes/audio_effect_filter.hpp>
#include <godot_cpp/classes/audio_effect_low_pass_filter.hpp>
#include <godot_cpp/classes/audio_effect_high_pass_filter.hpp>
#include <godot_cpp/classes/audio_effect_band_pass_filter.hpp>
#include <godot_cpp/classes/audio_effect_notch_filter.hpp>
#include <godot_cpp/classes/audio_effect_band_limit_filter.hpp>
#include <godot_cpp/classes/audio_effect_low_shelf_filter.hpp>
#include <godot_cpp/classes/audio_effect_high_shelf_filter.hpp>
#include <godot_cpp/classes/audio_effect_eq.hpp>
#include <godot_cpp/classes/audio_effect_eq6.hpp>
#include <godot_cpp/classes/audio_effect_eq10.hpp>
#include <godot_cpp/classes/audio_effect_eq21.hpp>


///////////////////////////
// Vital: API: Audio_2D //
///////////////////////////

namespace Vital::Sandbox::API {
    struct Audio2D : vm_module {
        inline static const std::vector<std::string> base_scope = {"core", "audio_2d"};
        using base_class = Vital::Engine::Audio2D;

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
            vm_module::bind_method<Instance>(vm, "is_playing", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> is_playing());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_paused", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_player() -> get_stream_paused());
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
                vm_args(vm, id, "(volume)", true)
                    .require(2, &Machine::is_number);

                auto volume = vm -> get_float(2);
                self -> audio -> get_player() -> set_volume_linear(volume);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_paused", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(pause)", true)
                    .require(2, &Machine::is_bool);

                auto pause = vm -> get_bool(2);
                self -> audio -> get_player() -> set_stream_paused(pause);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_max_distance", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(pixels)", true)
                    .require(2, &Machine::is_number);

                auto pixels = vm -> get_float(2);
                self -> audio -> get_player() -> set_max_distance(pixels);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_attenuation", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(curve)", true)
                    .require(2, &Machine::is_number);

                auto curve = vm -> get_float(2);
                self -> audio -> get_player() -> set_attenuation(curve);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_pitch_scale", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(pitch_scale)", true)
                    .require(2, &Machine::is_number);

                auto pitch_scale = vm -> get_float(2);
                self -> audio -> get_player() -> set_pitch_scale(pitch_scale);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_panning_strength", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(strength)", true)
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

            // Effects //
            vm_module::bind_method<Instance>(vm, "add_effect", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(type, parameters = {})")
                    .require(2, &Machine::is_string)
                    .optional(3, &Machine::is_table);
            
                auto type = vm -> get_string(2);
                godot::Ref<godot::AudioEffect> effect;
            
                auto read_f = [&](const std::string& key, auto setter) {
                    vm -> get_table_field(key, 3);
                    if (vm -> is_number(-1)) setter(vm -> get_float(-1));
                    vm -> pop(1);
                };
                auto read_b = [&](const std::string& key, auto setter) {
                    vm -> get_table_field(key, 3);
                    if (vm -> is_bool(-1)) setter(vm -> get_bool(-1));
                    vm -> pop(1);
                };
                auto read_i = [&](const std::string& key, auto setter) {
                    vm -> get_table_field(key, 3);
                    if (vm -> is_number(-1)) setter(vm -> get_int(-1));
                    vm -> pop(1);
                };
                bool has_params = vm -> is_table(3);
            
                // Filter helper — shared by all AudioEffectFilter subclasses
                // NOTE: takes Ref<AudioEffectFilter>, not a raw pointer — Ref<Derived> converts
                // implicitly to Ref<Base>, but a raw Derived* does NOT convert to Ref<Base>.
                auto apply_filter_params = [&](const godot::Ref<godot::AudioEffectFilter>& f) {
                    if (!has_params) return;
                    read_f("cutoff",    [&](float v) { f -> set_cutoff(v); });
                    read_f("resonance", [&](float v) { f -> set_resonance(v); });
                    read_f("gain",      [&](float v) { f -> set_gain(v); });
                    read_i("db",        [&](int v)   { f -> set_db(static_cast<godot::AudioEffectFilter::FilterDB>(v)); });
                };
            
                // EQ helper — shared by EQ6, EQ10, EQ21
                auto apply_eq_params = [&](const godot::Ref<godot::AudioEffectEQ>& eq) {
                    if (!has_params) return;
                    vm -> get_table_field("bands", 3);
                    if (vm -> is_table(-1)) {
                        int band_count = eq -> get_band_count();
                        for (int i = 0; i < band_count; i++) {
                            vm -> get_table_field(i + 1, -1);
                            if (vm -> is_number(-1)) eq -> set_band_gain_db(i, vm -> get_float(-1));
                            vm -> pop(1);
                        }
                    }
                    vm -> pop(1);
                };
            
                if (type == "reverb") {
                    godot::Ref<godot::AudioEffectReverb> e;
                    e.instantiate();
                    if (has_params) {
                        read_f("room_size",          [&](float v) { e -> set_room_size(v); });
                        read_f("damping",            [&](float v) { e -> set_damping(v); });
                        read_f("spread",             [&](float v) { e -> set_spread(v); });
                        read_f("wet",                [&](float v) { e -> set_wet(v); });
                        read_f("dry",                [&](float v) { e -> set_dry(v); });
                        read_f("hpf",                [&](float v) { e -> set_hpf(v); });
                        read_f("predelay_msec",      [&](float v) { e -> set_predelay_msec(v); });
                        read_f("predelay_feedback",  [&](float v) { e -> set_predelay_feedback(v); });
                    }
                    effect = e;
                }
                else if (type == "chorus") {
                    godot::Ref<godot::AudioEffectChorus> e;
                    e.instantiate();
                    if (has_params) {
                        read_i("voice_count", [&](int v)   { e -> set_voice_count(v); });
                        read_f("wet",         [&](float v) { e -> set_wet(v); });
                        read_f("dry",         [&](float v) { e -> set_dry(v); });
                        vm -> get_table_field("voices", 3);
                        if (vm -> is_table(-1)) {
                            for (int i = 0; i < e -> get_voice_count(); i++) {
                                vm -> get_table_field(i + 1, -1);
                                if (vm -> is_table(-1)) {
                                    vm -> get_table_field("delay_ms",  -1); if (vm -> is_number(-1)) e -> set_voice_delay_ms(i, vm -> get_float(-1));  vm -> pop(1);
                                    vm -> get_table_field("rate_hz",   -1); if (vm -> is_number(-1)) e -> set_voice_rate_hz(i, vm -> get_float(-1));   vm -> pop(1);
                                    vm -> get_table_field("depth_ms",  -1); if (vm -> is_number(-1)) e -> set_voice_depth_ms(i, vm -> get_float(-1));  vm -> pop(1);
                                    vm -> get_table_field("level_db",  -1); if (vm -> is_number(-1)) e -> set_voice_level_db(i, vm -> get_float(-1));  vm -> pop(1);
                                    vm -> get_table_field("cutoff_hz", -1); if (vm -> is_number(-1)) e -> set_voice_cutoff_hz(i, vm -> get_float(-1)); vm -> pop(1);
                                    vm -> get_table_field("pan",       -1); if (vm -> is_number(-1)) e -> set_voice_pan(i, vm -> get_float(-1));       vm -> pop(1);
                                }
                                vm -> pop(1);
                            }
                        }
                        vm -> pop(1);
                    }
                    effect = e;
                }
                else if (type == "delay") {
                    godot::Ref<godot::AudioEffectDelay> e;
                    e.instantiate();
                    if (has_params) {
                        read_f("dry",                [&](float v) { e -> set_dry(v); });
                        read_b("tap1_active",        [&](bool v)  { e -> set_tap1_active(v); });
                        read_f("tap1_delay_ms",      [&](float v) { e -> set_tap1_delay_ms(v); });
                        read_f("tap1_level_db",      [&](float v) { e -> set_tap1_level_db(v); });
                        read_f("tap1_pan",           [&](float v) { e -> set_tap1_pan(v); });
                        read_b("tap2_active",        [&](bool v)  { e -> set_tap2_active(v); });
                        read_f("tap2_delay_ms",      [&](float v) { e -> set_tap2_delay_ms(v); });
                        read_f("tap2_level_db",      [&](float v) { e -> set_tap2_level_db(v); });
                        read_f("tap2_pan",           [&](float v) { e -> set_tap2_pan(v); });
                        read_b("feedback_active",    [&](bool v)  { e -> set_feedback_active(v); });
                        read_f("feedback_delay_ms",  [&](float v) { e -> set_feedback_delay_ms(v); });
                        read_f("feedback_level_db",  [&](float v) { e -> set_feedback_level_db(v); });
                        read_f("feedback_lowpass",   [&](float v) { e -> set_feedback_lowpass(v); });
                    }
                    effect = e;
                }
                else if (type == "distortion") {
                    godot::Ref<godot::AudioEffectDistortion> e;
                    e.instantiate();
                    if (has_params) {
                        read_i("mode",        [&](int v)   { e -> set_mode(static_cast<godot::AudioEffectDistortion::Mode>(v)); });
                        read_f("pre_gain",    [&](float v) { e -> set_pre_gain(v); });
                        read_f("keep_hf_hz",  [&](float v) { e -> set_keep_hf_hz(v); });
                        read_f("drive",       [&](float v) { e -> set_drive(v); });
                        read_f("post_gain",   [&](float v) { e -> set_post_gain(v); });
                    }
                    effect = e;
                }
                else if (type == "amplify") {
                    godot::Ref<godot::AudioEffectAmplify> e;
                    e.instantiate();
                    if (has_params) {
                        read_f("volume_db",     [&](float v) { e -> set_volume_db(v); });
                        read_f("volume_linear", [&](float v) { e -> set_volume_linear(v); });
                    }
                    effect = e;
                }
                else if (type == "compressor") {
                    godot::Ref<godot::AudioEffectCompressor> e;
                    e.instantiate();
                    if (has_params) {
                        read_f("threshold",  [&](float v) { e -> set_threshold(v); });
                        read_f("ratio",      [&](float v) { e -> set_ratio(v); });
                        read_f("gain",       [&](float v) { e -> set_gain(v); });
                        read_f("attack_us",  [&](float v) { e -> set_attack_us(v); });
                        read_f("release_ms", [&](float v) { e -> set_release_ms(v); });
                        read_f("mix",        [&](float v) { e -> set_mix(v); });
                    }
                    effect = e;
                }
                else if (type == "limiter") {
                    godot::Ref<godot::AudioEffectLimiter> e;
                    e.instantiate();
                    if (has_params) {
                        read_f("ceiling_db",      [&](float v) { e -> set_ceiling_db(v); });
                        read_f("threshold_db",    [&](float v) { e -> set_threshold_db(v); });
                        read_f("soft_clip_db",    [&](float v) { e -> set_soft_clip_db(v); });
                        read_f("soft_clip_ratio", [&](float v) { e -> set_soft_clip_ratio(v); });
                    }
                    effect = e;
                }
                else if (type == "hard_limiter") {
                    godot::Ref<godot::AudioEffectHardLimiter> e;
                    e.instantiate();
                    if (has_params) {
                        read_f("ceiling_db",  [&](float v) { e -> set_ceiling_db(v); });
                        read_f("pre_gain_db", [&](float v) { e -> set_pre_gain_db(v); });
                        read_f("release",     [&](float v) { e -> set_release(v); });
                    }
                    effect = e;
                }
                else if (type == "panner") {
                    godot::Ref<godot::AudioEffectPanner> e;
                    e.instantiate();
                    if (has_params) {
                        read_f("pan", [&](float v) { e -> set_pan(v); });
                    }
                    effect = e;
                }
                else if (type == "phaser") {
                    godot::Ref<godot::AudioEffectPhaser> e;
                    e.instantiate();
                    if (has_params) {
                        read_f("range_min_hz", [&](float v) { e -> set_range_min_hz(v); });
                        read_f("range_max_hz", [&](float v) { e -> set_range_max_hz(v); });
                        read_f("rate_hz",      [&](float v) { e -> set_rate_hz(v); });
                        read_f("feedback",     [&](float v) { e -> set_feedback(v); });
                        read_f("depth",        [&](float v) { e -> set_depth(v); });
                    }
                    effect = e;
                }
                else if (type == "pitch_shift") {
                    godot::Ref<godot::AudioEffectPitchShift> e;
                    e.instantiate();
                    if (has_params) {
                        read_f("pitch_scale",  [&](float v) { e -> set_pitch_scale(v); });
                        read_i("oversampling", [&](int v)   { e -> set_oversampling(v); });
                        read_i("fft_size",     [&](int v)   { e -> set_fft_size(static_cast<godot::AudioEffectPitchShift::FFTSize>(v)); });
                    }
                    effect = e;
                }
                else if (type == "stereo_enhance") {
                    godot::Ref<godot::AudioEffectStereoEnhance> e;
                    e.instantiate();
                    if (has_params) {
                        read_f("pan_pullout",  [&](float v) { e -> set_pan_pullout(v); });
                        read_f("time_pullout", [&](float v) { e -> set_time_pullout(v); });
                        read_f("surround",     [&](float v) { e -> set_surround(v); });
                    }
                    effect = e;
                }
                else if (type == "lowpass_filter") {
                    godot::Ref<godot::AudioEffectLowPassFilter> e;
                    e.instantiate();
                    apply_filter_params(e);
                    effect = e;
                }
                else if (type == "highpass_filter") {
                    godot::Ref<godot::AudioEffectHighPassFilter> e;
                    e.instantiate();
                    apply_filter_params(e);
                    effect = e;
                }
                else if (type == "bandpass_filter") {
                    godot::Ref<godot::AudioEffectBandPassFilter> e;
                    e.instantiate();
                    apply_filter_params(e);
                    effect = e;
                }
                else if (type == "notch_filter") {
                    godot::Ref<godot::AudioEffectNotchFilter> e;
                    e.instantiate();
                    apply_filter_params(e);
                    effect = e;
                }
                else if (type == "bandlimit_filter") {
                    godot::Ref<godot::AudioEffectBandLimitFilter> e;
                    e.instantiate();
                    apply_filter_params(e);
                    effect = e;
                }
                else if (type == "lowshelf_filter") {
                    godot::Ref<godot::AudioEffectLowShelfFilter> e;
                    e.instantiate();
                    apply_filter_params(e);
                    effect = e;
                }
                else if (type == "highshelf_filter") {
                    godot::Ref<godot::AudioEffectHighShelfFilter> e;
                    e.instantiate();
                    apply_filter_params(e);
                    effect = e;
                }
                else if (type == "eq6") {
                    godot::Ref<godot::AudioEffectEQ6> e;
                    e.instantiate();
                    apply_eq_params(e);
                    effect = e;
                }
                else if (type == "eq10") {
                    godot::Ref<godot::AudioEffectEQ10> e;
                    e.instantiate();
                    apply_eq_params(e);
                    effect = e;
                }
                else if (type == "eq21") {
                    godot::Ref<godot::AudioEffectEQ21> e;
                    e.instantiate();
                    apply_eq_params(e);
                    effect = e;
                }
            
                if (!effect.is_valid()) {
                    vm -> push_value(false);
                    return 1;
                }
                auto result = self -> audio -> add_effect(effect);
                if (result) vm -> push_value(self -> audio -> get_effect_count() - 1);
                else vm -> push_value(false);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "remove_effect", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(effect_index)", true)
                    .require(2, &Machine::is_number);

                auto effect_index = vm -> get_int(2);
                vm -> push_value(self -> audio -> remove_effect(effect_index));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_effect_count", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> audio -> get_effect_count());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_effect_enabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(effect_index, state)", true)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_bool);

                auto effect_index = vm -> get_int(2);
                auto state = vm -> get_bool(3);
                vm -> push_value(self -> audio -> set_effect_enabled(effect_index, state));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_effect_enabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(effect_index)", true)
                    .require(2, &Machine::is_number);

                auto effect_index = vm -> get_int(2);
                vm -> push_value(self -> audio -> is_effect_enabled(effect_index));
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
    struct Audio2D : vm_module {};
}
#endif