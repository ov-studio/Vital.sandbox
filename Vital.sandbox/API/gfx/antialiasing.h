/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: gfx: antialiasing.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Antialiasing APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>


///////////////////////////////
// Vital: API: Antialiasing //
///////////////////////////////

namespace Vital::Sandbox::API {
    struct Antialiasing : vm_module {
        inline static const std::vector<std::string> base_scope = {"gfx", "antialiasing"};
        using base_class = Vital::Engine::Core;

        enum Mode { 
            MODE_DISABLED, 
            MODE_FXAA, 
            MODE_TAA 
        };

        inline static const std::vector<std::pair<std::string, int>> aa_mode_registry = {
            { "DISABLED", MODE_DISABLED },
            { "FXAA",     MODE_FXAA     },
            { "TAA",      MODE_TAA      }
        };

        inline static const std::vector<std::pair<std::string, int>> msaa_mode_registry = {
            { "DISABLED", godot::Viewport::MSAA_DISABLED },
            { "X2",       godot::Viewport::MSAA_2X       },
            { "X4",       godot::Viewport::MSAA_4X       },
            { "X8",       godot::Viewport::MSAA_8X       }
        };

        struct Snapshot {
            bool taa;
            godot::Viewport::ScreenSpaceAA ssaa;
            godot::Viewport::MSAA msaa;
            float scaling_3d;

            static Snapshot capture(godot::Viewport* root) {
                return {
                    root -> is_using_taa(),
                    root -> get_screen_space_aa(),
                    root -> get_msaa_3d(),
                    root -> get_scaling_3d_scale()
                };
            }

            void apply(godot::Viewport* root) const {
                root -> set_use_taa(taa);
                root -> set_screen_space_aa(ssaa);
                root -> set_msaa_3d(msaa);
                root -> set_scaling_3d_scale(scaling_3d);
            }
        };
        inline static std::optional<Snapshot> original;

        static void init(Machine* vm) {
            if (!original) original = Snapshot::capture(base_class::get_scene_root());

            static Tool::Event::Handle reset_binding;
            reset_binding.bind("environment:free", [](Tool::Stack) {
                if (original) original -> apply(base_class::get_scene_root());
            });
        }

        static int get_mode() {
            auto root = base_class::get_scene_root();
            if (root -> is_using_taa()) return MODE_TAA;
            if (root -> get_screen_space_aa() == godot::Viewport::SCREEN_SPACE_AA_FXAA) return MODE_FXAA;
            return MODE_DISABLED;
        }

        static void set_mode(int mode) {
            auto root = base_class::get_scene_root();
            root -> set_use_taa(mode == MODE_TAA);
            root -> set_screen_space_aa(mode == MODE_FXAA ? godot::Viewport::SCREEN_SPACE_AA_FXAA : godot::Viewport::SCREEN_SPACE_AA_DISABLED);
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "get_mode", [](auto vm, auto& id) -> int {
                vm -> push_value(get_mode());
                return 1;
            });

            API::bind(vm, base_scope, "get_msaa_mode", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(base_class::get_scene_root() -> get_msaa_3d()));
                return 1;
            });

            API::bind(vm, base_scope, "get_scaling_3d_scale", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_scene_root() -> get_scaling_3d_scale());
                return 1;
            });

            API::bind(vm, base_scope, "set_mode", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(mode)")
                    .require_enum(1, aa_mode_registry);

                auto mode = vm -> get_int(1);
                set_mode(mode);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_msaa_mode", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(mode)")
                    .require_enum(1, msaa_mode_registry);

                auto mode = static_cast<godot::Viewport::MSAA>(vm -> get_int(1));
                base_class::get_scene_root() -> set_msaa_3d(mode);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_scaling_3d_scale", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(scale)")
                    .require(1, &Machine::is_number);

                auto scale = vm -> get_float(1);
                base_class::get_scene_root() -> set_scaling_3d_scale(scale);
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "aa_mode", aa_mode_registry);
            vm -> scope_set_enum(base_scope, "msaa_mode", msaa_mode_registry);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Antialiasing : vm_module {};
}
#endif
