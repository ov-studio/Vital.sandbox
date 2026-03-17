/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: gfx: emissive.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Emissive APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.extension/Sandbox/index.h>


////////////////////////////////////
// Vital: Sandbox: API: Emissive //
////////////////////////////////////

namespace Vital::Sandbox::API {
    struct Emissive : vm_module {
        inline static const std::string base_name = "gfx";
        using base_class = Vital::Engine::Core;

        static void bind(Machine* vm) {
            API::bind(vm, {base_name, "emissive"}, "is_enabled", [](auto vm) -> int {
                vm -> push_value(base_class::get_environment() -> is_glow_enabled());
                return 1;
            });

            API::bind(vm, {base_name, "emissive"}, "set_enabled", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_bool(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_glow_enabled(state);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "set_level_intensity", [](auto vm) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_number(1)) || (!vm -> is_number(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto idx = vm -> get_int(1);
                auto value = vm -> get_float(2);
                base_class::get_environment() -> set_glow_level(idx, value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "get_level_intensity", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto idx = vm -> get_int(1);
                vm -> push_number(base_class::get_environment() -> get_glow_level(idx));
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "is_normalized", [](auto vm) -> int {
                vm -> push_value(base_class::get_environment() -> is_glow_normalized());
                return 1;
            });

            API::bind(vm, {base_name, "emissive"}, "set_normalized", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_bool(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(1);
                base_class::get_environment() -> set_glow_normalized(state);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "set_intensity", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_glow_intensity(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "get_intensity", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_glow_intensity());
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "set_strength", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_glow_strength(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "get_strength", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_glow_strength());
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "set_mix", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_glow_mix(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "get_mix", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_glow_mix());
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "set_bloom", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_glow_bloom(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "get_bloom", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_glow_bloom());
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "set_blend_mode", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_int(1);
                if ((value < godot::Environment::GLOW_BLEND_MODE_ADDITIVE) || (value > godot::Environment::GLOW_BLEND_MODE_MIX)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                base_class::get_environment() -> set_glow_blend_mode(static_cast<godot::Environment::GlowBlendMode>(value));
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "get_blend_mode", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_glow_blend_mode());
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "set_hdr_bleed_threshold", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_glow_hdr_bleed_threshold(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "get_hdr_bleed_threshold", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_glow_hdr_bleed_threshold());
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "set_hdr_bleed_scale", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_glow_hdr_bleed_scale(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "get_hdr_bleed_scale", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_glow_hdr_bleed_scale());
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "set_hdr_luminance_cap", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_glow_hdr_luminance_cap(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "get_hdr_luminance_cap", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_glow_hdr_luminance_cap());
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "set_map_strength", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(1);
                base_class::get_environment() -> set_glow_map_strength(value);
                vm -> push_value(true);
                return 1;
            });
        
            API::bind(vm, {base_name, "emissive"}, "get_map_strength", [](auto vm) -> int {
                vm -> push_number(base_class::get_environment() -> get_glow_map_strength());
                return 1;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Emissive : vm_module {};
}
#endif