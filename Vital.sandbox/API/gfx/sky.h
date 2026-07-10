/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: gfx: sky.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sky APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/texture.h>
#include <Vital.sandbox/API/utility/file.h>


//////////////////////
// Vital: API: Sky //
//////////////////////

namespace Vital::Sandbox::API {
    struct Sky : vm_module {
        inline static const std::vector<std::string> base_scope = {"gfx", "sky"};
        using base_class = Vital::Engine::Core;

        inline static const std::vector<std::pair<std::string, int>> radiance_size_registry = {
            { "SIZE_32",   godot::Sky::RADIANCE_SIZE_32   },
            { "SIZE_64",   godot::Sky::RADIANCE_SIZE_64   },
            { "SIZE_128",  godot::Sky::RADIANCE_SIZE_128  },
            { "SIZE_256",  godot::Sky::RADIANCE_SIZE_256  },
            { "SIZE_512",  godot::Sky::RADIANCE_SIZE_512  },
            { "SIZE_1024", godot::Sky::RADIANCE_SIZE_1024 },
            { "SIZE_2048", godot::Sky::RADIANCE_SIZE_2048 }
        };

        inline static const std::vector<std::pair<std::string, int>> process_mode_registry = {
            { "AUTOMATIC",   godot::Sky::PROCESS_MODE_AUTOMATIC   },
            { "QUALITY",     godot::Sky::PROCESS_MODE_QUALITY     },
            { "INCREMENTAL", godot::Sky::PROCESS_MODE_INCREMENTAL },
            { "REALTIME",    godot::Sky::PROCESS_MODE_REALTIME    }
        };

        template<typename T>
        static godot::Ref<T> ensure_material() {
            auto sky = base_class::get_sky();
            godot::Ref<godot::Material> current = sky -> get_material();
            godot::Ref<T> typed = current;
            if (!typed.is_valid()) {
                typed.instantiate();
                sky -> set_material(typed);
            }
            return typed;
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "get_mode", [](auto vm, auto& id) -> int {
                godot::Ref<godot::Material> current = base_class::get_sky() -> get_material();
                if (godot::Ref<godot::PanoramaSkyMaterial>(current).is_valid()) vm -> push_value(std::string("panorama"));
                else if (godot::Ref<godot::ProceduralSkyMaterial>(current).is_valid()) vm -> push_value(std::string("procedural"));
                else if (godot::Ref<godot::PhysicalSkyMaterial>(current).is_valid()) vm -> push_value(std::string("physical"));
                else vm -> push_value(false);
                return 1;
            });

            API::bind(vm, base_scope, "get_fov", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_sky_custom_fov());
                return 1;
            });

            API::bind(vm, base_scope, "get_rotation", [](auto vm, auto& id) -> int {
                vm -> push_value(base_class::get_environment() -> get_sky_rotation());
                return 1;
            });

            API::bind(vm, base_scope, "get_radiance_size", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(base_class::get_sky() -> get_radiance_size()));
                return 1;
            });

            API::bind(vm, base_scope, "get_process_mode", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(base_class::get_sky() -> get_process_mode()));
                return 1;
            });

            API::bind(vm, base_scope, "set_mode", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(mode)")
                    .require(1, &Machine::is_string);

                auto type = vm -> get_string(1);
                auto sky = base_class::get_sky();
                if (type == "panorama") { godot::Ref<godot::PanoramaSkyMaterial> material; material.instantiate(); sky -> set_material(material); }
                else if (type == "procedural") { godot::Ref<godot::ProceduralSkyMaterial> material; material.instantiate(); sky -> set_material(material); }
                else if (type == "physical") { godot::Ref<godot::PhysicalSkyMaterial> material; material.instantiate(); sky -> set_material(material); }
                else throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: invalid sky type");
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_fov", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(1, &Machine::is_number);

                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_sky_custom_fov(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_rotation", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(rotation)")
                    .require(1, &Machine::is_vector3);

                auto value = vm -> get_vector3(1);
                base_class::get_environment() -> set_sky_rotation(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_radiance_size", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(size)")
                    .require_enum(1, radiance_size_registry);

                auto value = static_cast<godot::Sky::RadianceSize>(vm -> get_int(1));
                base_class::get_sky() -> set_radiance_size(value);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "set_process_mode", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(mode)")
                    .require_enum(1, process_mode_registry);

                auto value = static_cast<godot::Sky::ProcessMode>(vm -> get_int(1));
                base_class::get_sky() -> set_process_mode(value);
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "radiance_size", radiance_size_registry);
            vm -> scope_set_enum(base_scope, "process_mode", process_mode_registry);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Sky : vm_module {};
}
#endif