/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: texture.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Texture APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.extension/Sandbox/index.h>
#include <Vital.extension/Engine/public/texture.h>


///////////////////////////////////
// Vital: Sandbox: API: Texture //
///////////////////////////////////

namespace Vital::Sandbox::API {
    struct Texture : vm_module {
        inline static const std::string base_name = "texture";
        using base_class = Vital::Engine::Texture;

        static void bind(Machine* vm) {
            vm_module::register_type<Texture>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                auto object = base_class::create_texture_2d(path);
                vm -> create_object(base_name, object);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<base_class>(vm, base_name, "destroy", [](auto vm, auto self) -> int {
                self -> destroy();
                vm_module::release_userdata(vm, 1);
                vm -> push_value(true);
                return 1;
            });
        }
    };

    struct SVG : vm_module {
        inline static const std::string base_name = "svg";
        using base_class = Vital::Engine::Texture;

        static void bind(Machine* vm) {
            vm_module::register_type<SVG>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                auto object = base_class::create_svg(path);
                vm -> create_object(base_name, object);
                return 1;
            });

            API::bind(vm, {base_name}, "create_from_raw", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto raw = vm -> get_string(1);
                auto object = base_class::create_svg_from_raw(raw);
                vm -> create_object(base_name, object);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<base_class>(vm, base_name, "destroy", [](auto vm, auto self) -> int {
                self -> destroy();
                vm_module::release_userdata(vm, 1);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "update", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto raw = vm -> get_string(2);
                self -> update_svg_from_raw(raw);
                vm -> push_value(true);
                return 1;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Texture : vm_module {};
    struct SVG : vm_module {};
}
#endif