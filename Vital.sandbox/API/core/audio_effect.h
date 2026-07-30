/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: audio_effect.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Audio Effect APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
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


///////////////////////////////
// Vital: API: Audio_Effect //
///////////////////////////////

namespace Vital::Sandbox::API {
    struct Audio_Effect {
        enum class Effect {
            REVERB,
            CHORUS,
            DELAY,
            DISTORTION,
            AMPLIFY,
            COMPRESSOR,
            LIMITER,
            HARD_LIMITER,
            PANNER,
            PHASER,
            PITCH_SHIFT,
            STEREO_ENHANCE,
            LOWPASS_FILTER,
            HIGHPASS_FILTER,
            BANDPASS_FILTER,
            NOTCH_FILTER,
            BANDLIMIT_FILTER,
            LOWSHELF_FILTER,
            HIGHSHELF_FILTER,
            EQ6,
            EQ10,
            EQ21
        };

        inline static const std::vector<std::pair<std::string, Effect>> effect_registry = {
            {"REVERB",           Effect::REVERB           },
            {"CHORUS",           Effect::CHORUS           },
            {"DELAY",            Effect::DELAY            },
            {"DISTORTION",       Effect::DISTORTION       },
            {"AMPLIFY",          Effect::AMPLIFY          },
            {"COMPRESSOR",       Effect::COMPRESSOR       },
            {"LIMITER",          Effect::LIMITER          },
            {"HARD_LIMITER",     Effect::HARD_LIMITER     },
            {"PANNER",           Effect::PANNER           },
            {"PHASER",           Effect::PHASER           },
            {"PITCH_SHIFT",      Effect::PITCH_SHIFT      },
            {"STEREO_ENHANCE",   Effect::STEREO_ENHANCE   },
            {"LOWPASS_FILTER",   Effect::LOWPASS_FILTER   },
            {"HIGHPASS_FILTER",  Effect::HIGHPASS_FILTER  },
            {"BANDPASS_FILTER",  Effect::BANDPASS_FILTER  },
            {"NOTCH_FILTER",     Effect::NOTCH_FILTER     },
            {"BANDLIMIT_FILTER", Effect::BANDLIMIT_FILTER },
            {"LOWSHELF_FILTER",  Effect::LOWSHELF_FILTER  },
            {"HIGHSHELF_FILTER", Effect::HIGHSHELF_FILTER },
            {"EQ6",              Effect::EQ6              },
            {"EQ10",             Effect::EQ10             },
            {"EQ21",             Effect::EQ21             }
        };

        template<typename T, typename Fn>
        static godot::Ref<T> make(Fn&& configure) {
            godot::Ref<T> e;
            e.instantiate();
            configure(e);
            return e;
        }

        static godot::Ref<godot::AudioEffect> build(Machine* vm, Effect type, bool has_params, int param_idx) {
            godot::Ref<godot::AudioEffect> effect;

            auto read_bool = [&](const std::string& key, auto setter) {
                vm -> get_table_field(key, param_idx);
                if (vm -> is_bool(-1)) setter(vm -> get_bool(-1));
                vm -> pop(1);
            };

            auto read_int = [&](const std::string& key, auto setter) {
                vm -> get_table_field(key, param_idx);
                if (vm -> is_number(-1)) setter(vm -> get_int(-1));
                vm -> pop(1);
            };

            auto read_float = [&](const std::string& key, auto setter) {
                vm -> get_table_field(key, param_idx);
                if (vm -> is_number(-1)) setter(vm -> get_float(-1));
                vm -> pop(1);
            };

            auto apply_filter_params = [&](const godot::Ref<godot::AudioEffectFilter>& f) {
                if (!has_params) return;
                read_float("cutoff",    [&](float v) { f -> set_cutoff(v);                                              });
                read_float("resonance", [&](float v) { f -> set_resonance(v);                                           });
                read_float("gain",      [&](float v) { f -> set_gain(v);                                                });
                read_int("db",          [&](int v)   { f -> set_db(static_cast<godot::AudioEffectFilter::FilterDB>(v)); });
            };

            auto apply_eq_params = [&](const godot::Ref<godot::AudioEffectEQ>& eq) {
                if (!has_params) return;
                vm -> get_table_field("bands", param_idx);
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

            switch (type) {
                case Effect::REVERB: {
                    effect = make<godot::AudioEffectReverb>([&](auto& e) {
                        if (!has_params) return;
                        read_float("room_size",         [&](float v) { e -> set_room_size(v);         });
                        read_float("damping",           [&](float v) { e -> set_damping(v);           });
                        read_float("spread",            [&](float v) { e -> set_spread(v);            });
                        read_float("wet",               [&](float v) { e -> set_wet(v);               });
                        read_float("dry",               [&](float v) { e -> set_dry(v);               });
                        read_float("hpf",               [&](float v) { e -> set_hpf(v);               });
                        read_float("predelay_msec",     [&](float v) { e -> set_predelay_msec(v);     });
                        read_float("predelay_feedback", [&](float v) { e -> set_predelay_feedback(v); });
                    });
                    break;
                }
                case Effect::CHORUS: {
                    effect = make<godot::AudioEffectChorus>([&](auto& e) {
                        if (!has_params) return;
                        read_int("voice_count",   [&](int v)   { e -> set_voice_count(v); });
                        read_float("wet",         [&](float v) { e -> set_wet(v);         });
                        read_float("dry",         [&](float v) { e -> set_dry(v);         });
                        vm -> get_table_field("voices", param_idx);
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
                    });
                    break;
                }
                case Effect::DELAY: {
                    effect = make<godot::AudioEffectDelay>([&](auto& e) {
                        if (!has_params) return;
                        read_float("dry",               [&](float v) { e -> set_dry(v);                });
                        read_bool("tap1_active",        [&](bool v)  { e -> set_tap1_active(v);        });
                        read_float("tap1_delay_ms",     [&](float v) { e -> set_tap1_delay_ms(v);      });
                        read_float("tap1_level_db",     [&](float v) { e -> set_tap1_level_db(v);      });
                        read_float("tap1_pan",          [&](float v) { e -> set_tap1_pan(v);           });
                        read_bool("tap2_active",        [&](bool v)  { e -> set_tap2_active(v);        });
                        read_float("tap2_delay_ms",     [&](float v) { e -> set_tap2_delay_ms(v);      });
                        read_float("tap2_level_db",     [&](float v) { e -> set_tap2_level_db(v);      });
                        read_float("tap2_pan",          [&](float v) { e -> set_tap2_pan(v);           });
                        read_bool("feedback_active",    [&](bool v)  { e -> set_feedback_active(v);    });
                        read_float("feedback_delay_ms", [&](float v) { e -> set_feedback_delay_ms(v);  });
                        read_float("feedback_level_db", [&](float v) { e -> set_feedback_level_db(v);  });
                        read_float("feedback_lowpass",  [&](float v) { e -> set_feedback_lowpass(v);   });
                    });
                    break;
                }
                case Effect::DISTORTION: {
                    effect = make<godot::AudioEffectDistortion>([&](auto& e) {
                        if (!has_params) return;
                        read_int("mode",         [&](int v)   { e -> set_mode(static_cast<godot::AudioEffectDistortion::Mode>(v)); });
                        read_float("pre_gain",   [&](float v) { e -> set_pre_gain(v);                                              });
                        read_float("keep_hf_hz", [&](float v) { e -> set_keep_hf_hz(v);                                            });
                        read_float("drive",      [&](float v) { e -> set_drive(v);                                                 });
                        read_float("post_gain",  [&](float v) { e -> set_post_gain(v);                                             });
                    });
                    break;
                }
                case Effect::AMPLIFY: {
                    effect = make<godot::AudioEffectAmplify>([&](auto& e) {
                        if (!has_params) return;
                        read_float("volume_db",     [&](float v) { e -> set_volume_db(v);     });
                        read_float("volume_linear", [&](float v) { e -> set_volume_linear(v); });
                    });
                    break;
                }
                case Effect::COMPRESSOR: {
                    effect = make<godot::AudioEffectCompressor>([&](auto& e) {
                        if (!has_params) return;
                        read_float("threshold",  [&](float v) { e -> set_threshold(v);  });
                        read_float("ratio",      [&](float v) { e -> set_ratio(v);      });
                        read_float("gain",       [&](float v) { e -> set_gain(v);       });
                        read_float("attack_us",  [&](float v) { e -> set_attack_us(v);  });
                        read_float("release_ms", [&](float v) { e -> set_release_ms(v); });
                        read_float("mix",        [&](float v) { e -> set_mix(v);        });
                    });
                    break;
                }
                case Effect::LIMITER: {
                    effect = make<godot::AudioEffectLimiter>([&](auto& e) {
                        if (!has_params) return;
                        read_float("ceiling_db",      [&](float v) { e -> set_ceiling_db(v);      });
                        read_float("threshold_db",    [&](float v) { e -> set_threshold_db(v);    });
                        read_float("soft_clip_db",    [&](float v) { e -> set_soft_clip_db(v);    });
                        read_float("soft_clip_ratio", [&](float v) { e -> set_soft_clip_ratio(v); });
                    });
                    break;
                }
                case Effect::HARD_LIMITER: {
                    effect = make<godot::AudioEffectHardLimiter>([&](auto& e) {
                        if (!has_params) return;
                        read_float("ceiling_db",  [&](float v) { e -> set_ceiling_db(v);  });
                        read_float("pre_gain_db", [&](float v) { e -> set_pre_gain_db(v); });
                        read_float("release",     [&](float v) { e -> set_release(v);     });
                    });
                    break;
                }
                case Effect::PANNER: {
                    effect = make<godot::AudioEffectPanner>([&](auto& e) {
                        if (!has_params) return;
                        read_float("pan", [&](float v) { e -> set_pan(v); });
                    });
                    break;
                }
                case Effect::PHASER: {
                    effect = make<godot::AudioEffectPhaser>([&](auto& e) {
                        if (!has_params) return;
                        read_float("range_min_hz", [&](float v) { e -> set_range_min_hz(v); });
                        read_float("range_max_hz", [&](float v) { e -> set_range_max_hz(v); });
                        read_float("rate_hz",      [&](float v) { e -> set_rate_hz(v);      });
                        read_float("feedback",     [&](float v) { e -> set_feedback(v);     });
                        read_float("depth",        [&](float v) { e -> set_depth(v);        });
                    });
                    break;
                }
                case Effect::PITCH_SHIFT: {
                    effect = make<godot::AudioEffectPitchShift>([&](auto& e) {
                        if (!has_params) return;
                        read_float("pitch_scale", [&](float v) { e -> set_pitch_scale(v);                                                  });
                        read_int("oversampling",  [&](int v)   { e -> set_oversampling(v);                                                 });
                        read_int("fft_size",      [&](int v)   { e -> set_fft_size(static_cast<godot::AudioEffectPitchShift::FFTSize>(v)); });
                    });
                    break;
                }
                case Effect::STEREO_ENHANCE: {
                    effect = make<godot::AudioEffectStereoEnhance>([&](auto& e) {
                        if (!has_params) return;
                        read_float("pan_pullout",  [&](float v) { e -> set_pan_pullout(v);  });
                        read_float("time_pullout", [&](float v) { e -> set_time_pullout(v); });
                        read_float("surround",     [&](float v) { e -> set_surround(v);     });
                    });
                    break;
                }
                case Effect::LOWPASS_FILTER:   effect = make<godot::AudioEffectLowPassFilter>(apply_filter_params);   break;
                case Effect::HIGHPASS_FILTER:  effect = make<godot::AudioEffectHighPassFilter>(apply_filter_params);  break;
                case Effect::BANDPASS_FILTER:  effect = make<godot::AudioEffectBandPassFilter>(apply_filter_params);  break;
                case Effect::NOTCH_FILTER:     effect = make<godot::AudioEffectNotchFilter>(apply_filter_params);     break;
                case Effect::BANDLIMIT_FILTER: effect = make<godot::AudioEffectBandLimitFilter>(apply_filter_params); break;
                case Effect::LOWSHELF_FILTER:  effect = make<godot::AudioEffectLowShelfFilter>(apply_filter_params);  break;
                case Effect::HIGHSHELF_FILTER: effect = make<godot::AudioEffectHighShelfFilter>(apply_filter_params); break;
                case Effect::EQ6:              effect = make<godot::AudioEffectEQ6>(apply_eq_params);                 break;
                case Effect::EQ10:             effect = make<godot::AudioEffectEQ10>(apply_eq_params);                break;
                case Effect::EQ21:             effect = make<godot::AudioEffectEQ21>(apply_eq_params);                break;
            }
            return effect;
        }

        template<typename Instance>
        static void bind(Machine* vm) {}

        template<typename Instance>
        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "is_fx_enabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name)", true)
                    .require(2, &Machine::is_string);

                vm -> push_value(self -> audio -> is_fx_enabled(vm -> get_string(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_fx_enabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name, state)", true)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_bool);

                vm -> push_value(self -> audio -> set_fx_enabled(vm -> get_string(2), vm -> get_bool(3)));
                return 1;
            });
            
            vm_module::bind_method<Instance>(vm, "add_fx", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name, effect, parameters = {})")
                    .require(2, &Machine::is_string)
                    .require_enum(3, effect_registry)
                    .optional(4, &Machine::is_table);

                auto name = vm -> get_string(2);
                auto effect = static_cast<Effect>(vm -> get_int(3));
                bool has_params = vm -> is_table(4);
                auto result = build(vm, effect, has_params, 4);
                if (!result.is_valid()) vm -> push_value(false);
                else vm -> push_value(self -> audio -> add_fx(name, result));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "remove_fx", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name)", true)
                    .require(2, &Machine::is_string);

                vm -> push_value(self -> audio -> remove_fx(vm -> get_string(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "list_fx", [](auto vm, auto self, auto& id) -> int {
                auto names = self -> audio -> list_effects();
                vm -> create_table();
                for (int i = 0; i < static_cast<int>(names.size()); i++) {
                    vm -> push_value(names[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });
        }

        template<typename Instance>
        static void inject(Machine* vm) {
            vm -> scope_set_enum(Instance::Owner::base_scope, "effect", effect_registry);
        }
    };
}
#endif