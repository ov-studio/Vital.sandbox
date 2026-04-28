/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: font.h
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
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/font.h>


///////////////////////
// Vital: API: Font //
///////////////////////

namespace Vital::Sandbox::API {
    struct Font : vm_module {
        inline static const std::string base_name = "font";
        using base_class = Vital::Engine::Font;

        static void bind(Machine* vm) {
            vm_module::register_type<Font>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path)")
                    .require(1, &Machine::is_string);

                auto path = vm -> get_string(1);
                auto object = base_class::create(path);
                vm -> create_object(base_name, object);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<base_class>(vm, base_name, "destroy", [](auto vm, auto self, auto& id) -> int {
                self -> destroy();
                vm_module::release_userdata(vm, 1);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_antialiasing", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(2, &Machine::is_bool);

                self -> set_antialiasing(vm -> get_bool(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_antialiasing", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> get_antialiasing());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_oversampling", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(2, &Machine::is_number);

                self -> set_oversampling(static_cast<float>(vm -> get_float(2)));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_oversampling", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> get_oversampling());
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