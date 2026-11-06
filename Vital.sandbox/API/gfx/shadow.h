/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: gfx: shadow.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 06/11/2026
     Desc: Shadow APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <godot_cpp/classes/project_settings.hpp>
#include <Vital.sandbox/Manager/public/sandbox.h>


/////////////////////////
// Vital: API: Shadow //
/////////////////////////

// TODO: Improve
namespace Vital::Sandbox::API {
    struct Shadow : vm_module {
        inline static const std::vector<std::string> base_scope = {"gfx", "shadow"};
        using base_class = Vital::Engine::Core;

        inline static constexpr int ATLAS_SIZE_MIN = 256;
        inline static constexpr int ATLAS_SIZE_MAX = 16384;
        inline static constexpr int QUADRANT_COUNT = 4;

        inline static const std::vector<std::pair<std::string, int>> quality_registry = {
            { "HARD",      godot::RenderingServer::SHADOW_QUALITY_HARD          },
            { "VERY_LOW",  godot::RenderingServer::SHADOW_QUALITY_SOFT_VERY_LOW },
            { "LOW",       godot::RenderingServer::SHADOW_QUALITY_SOFT_LOW      },
            { "MEDIUM",    godot::RenderingServer::SHADOW_QUALITY_SOFT_MEDIUM   },
            { "HIGH",      godot::RenderingServer::SHADOW_QUALITY_SOFT_HIGH     },
            { "ULTRA",     godot::RenderingServer::SHADOW_QUALITY_SOFT_ULTRA    }
        };

        inline static const std::vector<std::pair<std::string, int>> subdiv_registry = {
            { "DISABLED", godot::RenderingServer::SHADOW_ATLAS_QUADRANT_SUBDIV_DISABLED },
            { "X1",       godot::RenderingServer::SHADOW_ATLAS_QUADRANT_SUBDIV_1        },
            { "X4",       godot::RenderingServer::SHADOW_ATLAS_QUADRANT_SUBDIV_4        },
            { "X16",      godot::RenderingServer::SHADOW_ATLAS_QUADRANT_SUBDIV_16       },
            { "X64",      godot::RenderingServer::SHADOW_ATLAS_QUADRANT_SUBDIV_64       },
            { "X256",     godot::RenderingServer::SHADOW_ATLAS_QUADRANT_SUBDIV_256      },
            { "X1024",    godot::RenderingServer::SHADOW_ATLAS_QUADRANT_SUBDIV_1024     }
        };

        // RenderingServer exposes setters only, so the active values are tracked here //
        struct State {
            int directional_size = 4096;
            bool directional_16bits = true;
            int directional_quality = godot::RenderingServer::SHADOW_QUALITY_SOFT_LOW;
            int positional_size = 4096;
            bool positional_16bits = true;
            int positional_quality = godot::RenderingServer::SHADOW_QUALITY_SOFT_LOW;
            std::array<int, QUADRANT_COUNT> positional_subdiv = {
                godot::RenderingServer::SHADOW_ATLAS_QUADRANT_SUBDIV_4,
                godot::RenderingServer::SHADOW_ATLAS_QUADRANT_SUBDIV_4,
                godot::RenderingServer::SHADOW_ATLAS_QUADRANT_SUBDIV_16,
                godot::RenderingServer::SHADOW_ATLAS_QUADRANT_SUBDIV_64
            };
            int canvas_size = 2048;

            static State capture(godot::Viewport* root) {
                State state;
                auto settings = godot::ProjectSettings::get_singleton();
                if (settings) {
                    state.directional_size = (int)settings -> get_setting("rendering/lights_and_shadows/directional_shadow/size", state.directional_size);
                    state.directional_16bits = (bool)settings -> get_setting("rendering/lights_and_shadows/directional_shadow/16_bits", state.directional_16bits);
                    state.directional_quality = (int)settings -> get_setting("rendering/lights_and_shadows/directional_shadow/soft_shadow_filter_quality", state.directional_quality);
                    state.positional_quality = (int)settings -> get_setting("rendering/lights_and_shadows/positional_shadow/soft_shadow_filter_quality", state.positional_quality);
                    state.canvas_size = (int)settings -> get_setting("rendering/2d/shadow_atlas/size", state.canvas_size);
                }
                state.positional_size = root -> get_positional_shadow_atlas_size();
                state.positional_16bits = root -> get_positional_shadow_atlas_16_bits();
                for (int i = 0; i < QUADRANT_COUNT; i++) {
                    state.positional_subdiv[i] = static_cast<int>(root -> get_positional_shadow_atlas_quadrant_subdiv(i));
                }
                return state;
            }
        };
        inline static State current;
        inline static std::optional<State> original;

        static godot::RID get_viewport_rid() {
            return base_class::get_scene_root() -> get_viewport_rid();
        }

        static int round_size(int size) {
            int result = ATLAS_SIZE_MIN;
            size = std::clamp(size, ATLAS_SIZE_MIN, ATLAS_SIZE_MAX);
            while (result < size) result <<= 1;
            return result;
        }

        static void apply_directional_atlas() {
            base_class::get_rendering_server() -> directional_shadow_atlas_set_size(current.directional_size, current.directional_16bits);
        }

        static void apply_directional_quality() {
            base_class::get_rendering_server() -> directional_soft_shadow_filter_set_quality(static_cast<godot::RenderingServer::ShadowQuality>(current.directional_quality));
        }

        static void apply_positional_atlas() {
            base_class::get_rendering_server() -> viewport_set_positional_shadow_atlas_size(get_viewport_rid(), current.positional_size, current.positional_16bits);
        }

        static void apply_positional_subdiv(int quadrant) {
            base_class::get_rendering_server() -> viewport_set_positional_shadow_atlas_quadrant_subdivision(get_viewport_rid(), quadrant, static_cast<godot::RenderingServer::ShadowAtlasQuadrantSubdiv>(current.positional_subdiv[quadrant]));
        }

        static void apply_positional_quality() {
            base_class::get_rendering_server() -> positional_soft_shadow_filter_set_quality(static_cast<godot::RenderingServer::ShadowQuality>(current.positional_quality));
        }

        static void apply_canvas() {
            base_class::get_rendering_server() -> canvas_set_shadow_texture_size(current.canvas_size);
        }

        static void apply_all() {
            apply_directional_atlas();
            apply_directional_quality();
            apply_positional_atlas();
            for (int i = 0; i < QUADRANT_COUNT; i++) apply_positional_subdiv(i);
            apply_positional_quality();
            apply_canvas();
        }

        static void reset() {
            if (!original) return;
            current = *original;
            apply_all();
        }

        static void init(Machine* vm) {
            if (!original) {
                original = State::capture(base_class::get_scene_root());
                current = *original;
            }

            static Tool::Event::Handle reset_binding;
            reset_binding.bind("environment:free", [](Tool::Stack) {
                reset();
            });
        }

        static void bind(Machine* vm) {
            // Directional //
            API::bind(vm, base_scope, "get_directional_atlas_size", [](auto vm, auto& id) -> int {
                vm -> push_value(current.directional_size);
                return 1;
            });

            API::bind(vm, base_scope, "is_directional_atlas_16bits", [](auto vm, auto& id) -> int {
                vm -> push_value(current.directional_16bits);
                return 1;
            });

            API::bind(vm, base_scope, "get_directional_filter_quality", [](auto vm, auto& id) -> int {
                vm -> push_value(current.directional_quality);
                return 1;
            });

            API::bind(vm, base_scope, "set_directional_atlas_size", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(size, is_16bits = current)")
                    .require(1, &Machine::is_number)
                    .validate(1, [](Machine* vm, int idx) {
                        auto size = vm -> get_int(idx);
                        return size >= ATLAS_SIZE_MIN && size <= ATLAS_SIZE_MAX;
                    }, fmt::format("out of range [{} - {}]", ATLAS_SIZE_MIN, ATLAS_SIZE_MAX))
                    .optional(2, &Machine::is_bool);

                current.directional_size = round_size(vm -> get_int(1));
                if (vm -> is_bool(2)) current.directional_16bits = vm -> get_bool(2);
                apply_directional_atlas();
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_directional_filter_quality", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(quality)")
                    .require_enum(1, quality_registry);

                current.directional_quality = vm -> get_int(1);
                apply_directional_quality();
                vm -> push_value(true);
                return 1;
            });

            // Positional //
            API::bind(vm, base_scope, "get_positional_atlas_size", [](auto vm, auto& id) -> int {
                vm -> push_value(current.positional_size);
                return 1;
            });

            API::bind(vm, base_scope, "is_positional_atlas_16bits", [](auto vm, auto& id) -> int {
                vm -> push_value(current.positional_16bits);
                return 1;
            });

            API::bind(vm, base_scope, "get_positional_atlas_quadrant_subdiv", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(quadrant)")
                    .require(1, &Machine::is_number)
                    .validate(1, [](Machine* vm, int idx) {
                        auto quadrant = vm -> get_int(idx);
                        return quadrant >= 0 && quadrant < QUADRANT_COUNT;
                    }, fmt::format("out of range [0 - {}]", QUADRANT_COUNT - 1));

                vm -> push_value(current.positional_subdiv[vm -> get_int(1)]);
                return 1;
            });

            API::bind(vm, base_scope, "get_positional_filter_quality", [](auto vm, auto& id) -> int {
                vm -> push_value(current.positional_quality);
                return 1;
            });

            API::bind(vm, base_scope, "set_positional_atlas_size", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(size, is_16bits = current)")
                    .require(1, &Machine::is_number)
                    .validate(1, [](Machine* vm, int idx) {
                        auto size = vm -> get_int(idx);
                        return size >= ATLAS_SIZE_MIN && size <= ATLAS_SIZE_MAX;
                    }, fmt::format("out of range [{} - {}]", ATLAS_SIZE_MIN, ATLAS_SIZE_MAX))
                    .optional(2, &Machine::is_bool);

                current.positional_size = round_size(vm -> get_int(1));
                if (vm -> is_bool(2)) current.positional_16bits = vm -> get_bool(2);
                apply_positional_atlas();
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_positional_atlas_quadrant_subdiv", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(quadrant, subdiv)")
                    .require(1, &Machine::is_number)
                    .validate(1, [](Machine* vm, int idx) {
                        auto quadrant = vm -> get_int(idx);
                        return quadrant >= 0 && quadrant < QUADRANT_COUNT;
                    }, fmt::format("out of range [0 - {}]", QUADRANT_COUNT - 1))
                    .require_enum(2, subdiv_registry);

                auto quadrant = vm -> get_int(1);
                current.positional_subdiv[quadrant] = vm -> get_int(2);
                apply_positional_subdiv(quadrant);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_positional_filter_quality", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(quality)")
                    .require_enum(1, quality_registry);

                current.positional_quality = vm -> get_int(1);
                apply_positional_quality();
                vm -> push_value(true);
                return 1;
            });

            // Canvas //
            API::bind(vm, base_scope, "get_canvas_texture_size", [](auto vm, auto& id) -> int {
                vm -> push_value(current.canvas_size);
                return 1;
            });

            API::bind(vm, base_scope, "set_canvas_texture_size", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(size)")
                    .require(1, &Machine::is_number)
                    .validate(1, [](Machine* vm, int idx) {
                        auto size = vm -> get_int(idx);
                        return size >= ATLAS_SIZE_MIN && size <= ATLAS_SIZE_MAX;
                    }, fmt::format("out of range [{} - {}]", ATLAS_SIZE_MIN, ATLAS_SIZE_MAX));

                current.canvas_size = round_size(vm -> get_int(1));
                apply_canvas();
                vm -> push_value(true);
                return 1;
            });

            // Misc //
            API::bind(vm, base_scope, "reset", [](auto vm, auto& id) -> int {
                reset();
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "quality", quality_registry);
            vm -> scope_set_enum(base_scope, "subdiv", subdiv_registry);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Shadow : vm_module {};
}
#endif
