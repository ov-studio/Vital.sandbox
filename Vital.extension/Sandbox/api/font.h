/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: font.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Font APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.extension/Sandbox/index.h>
#include <Vital.extension/Engine/public/font.h>


////////////////////////////////
// Vital: Sandbox: API: Font //
////////////////////////////////

namespace Vital::Sandbox::API {
    struct Font : vm_module {
        inline static const std::string base_name = "font";
        using base_class = Vital::Engine::Font;

        static void bind(Machine* vm) {
            vm_module::register_type<Font>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto path = vm -> get_string(1);
                auto object = Vital::Engine::Font::create(path);
                vm -> create_object(base_name, object);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<base_class>(vm, base_name, "destroy", [](auto vm, auto self) -> int {
                self -> destroy();
                vm_module::release_userdata(vm, 1);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_antialiasing", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_bool(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                self -> set_antialiasing(vm -> get_bool(2));
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_antialiasing", [](auto vm, auto self) -> int {
                vm -> push_bool(self -> get_antialiasing());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_oversampling", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_number(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                self -> set_oversampling(static_cast<float>(vm -> get_float(2)));
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_oversampling", [](auto vm, auto self) -> int {
                vm -> push_number(self -> get_oversampling());
                return 1;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Font : vm_module {};
}
#endif