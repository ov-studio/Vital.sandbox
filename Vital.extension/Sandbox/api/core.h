/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: api: core.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Core APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.extension/Sandbox/index.h>
#include <Vital.extension/Engine/public/core.h>


////////////////////////////////
// Vital: Sandbox: API: Core //
////////////////////////////////

namespace Vital::Sandbox::API {
    struct Core : vm_module {
        inline static const std::string base_name = "engine";

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "get_screen_position_from_world", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_vector3(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto position = vm -> get_vector3(1);
                auto padding = vm -> is_number(2) ? vm -> get_float(2) : 0.0f;
                auto result = Vital::Engine::Core::get_singleton() -> get_screen_position_from_world(position, padding);
                vm -> push_vector2({result.x, result.y});
                vm -> push_number(result.z);
                return 2;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Canvas : vm_module {};
}
#endif