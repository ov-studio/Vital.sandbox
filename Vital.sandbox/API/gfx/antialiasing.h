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

        inline static const std::vector<std::pair<std::string, int>> mode_registry = {
            { "DISABLED", MODE_DISABLED },
            { "FXAA",     MODE_FXAA     },
            { "TAA",      MODE_TAA      }
        };

        inline static const std::vector<std::pair<std::string, int>> msaa_registry = {
            { "DISABLED", godot::Viewport::MSAA_DISABLED },
            { "X2",       godot::Viewport::MSAA_2X       },
            { "X4",       godot::Viewport::MSAA_4X       },
            { "X8",       godot::Viewport::MSAA_8X       }
        };

        static int get_mode() {
            auto root = base_class::get_scene_root();
            if (root -> is_using_taa()) return MODE_TAA;
            if (root -> get_screen_space_aa() == godot::Viewport::SCREEN_SPACE_AA_FXAA) return MODE_FXAA;
            return MODE_DISABLED;
        }

        static void set_mode(int mode) {
            auto root = base_class::get_scene_root();
            switch (mode) {
                case MODE_FXAA:
                    root -> set_use_taa(false);
                    root -> set_screen_space_aa(godot::Viewport::SCREEN_SPACE_AA_FXAA);
                    break;
                case MODE_TAA:
                    root -> set_use_taa(true);
                    root -> set_screen_space_aa(godot::Viewport::SCREEN_SPACE_AA_DISABLED);
                    break;
                default:
                    root -> set_use_taa(false);
                    root -> set_screen_space_aa(godot::Viewport::SCREEN_SPACE_AA_DISABLED);
                    break;
            }
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "get_mode", [](auto vm, auto& id) -> int {
                vm -> push_value(get_mode());
                return 1;
            });

            API::bind(vm, base_scope, "set_mode", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(mode)")
                    .require_enum(1, mode_registry);

                set_mode(vm -> get_int(1));
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_msaa_mode", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(base_class::get_scene_root() -> get_msaa_3d()));
                return 1;
            });

            API::bind(vm, base_scope, "set_msaa_mode", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(mode)")
                    .require_enum(1, msaa_registry);

                auto mode = static_cast<godot::Viewport::MSAA>(vm -> get_int(1));
                base_class::get_scene_root() -> set_msaa_3d(mode);
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "mode", mode_registry);
            vm -> scope_set_enum(base_scope, "msaa_mode", msaa_registry);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Antialiasing : vm_module {};
}
#endif