/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: rendertarget.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rendertarget APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/rendertarget.h>


///////////////////////////////
// Vital: API: Rendertarget //
///////////////////////////////

namespace Vital::Sandbox::API {
    struct Rendertarget : vm_module {
        inline static const std::string base_name = "rendertarget";
        using base_class = Vital::Engine::Rendertarget;

        static void bind(Machine* vm) {
            vm_module::register_type<Rendertarget>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(width, height, transparent)")
                    .require(1, &Machine::is_number)
                    .require(2, &Machine::is_number);

                auto width = vm -> get_int(1);
                auto height = vm -> get_int(2);
                auto transparent = vm -> is_bool(3) ? vm -> get_bool(3) : false;
                auto object = base_class::create(width, height, transparent);
                vm -> create_object(base_name, object);
                return 1;
            });

            API::bind(vm, {base_name}, "set_active", [](auto vm, auto& id) -> int {
                base_class* rt = nullptr;
                if (vm -> get_count() >= 1 && !vm -> is_nil(1)) {
                    vm_args(vm, id, "(rendertarget, clear, instant)")
                        .require(1, [](Machine* vm, int index) {
                            return vm -> is_userdata(index) && vm_module::is_userdata<Vital::Engine::Rendertarget>(vm, Rendertarget::base_name, index);
                        });
        
                    rt = static_cast<base_class*>(vm -> get_userdata(1));
                }
                auto clear = vm -> is_bool(2) ? vm -> get_bool(2) : false;
                auto instant = vm -> is_bool(3) ? vm -> get_bool(3) : false;
                base_class::set_active(rt, clear, instant);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_active", [](auto vm, auto& id) -> int {
                auto object = base_class::get_active();
                if (!object) vm -> push_value(false);
                else vm -> create_object(base_name, object);
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

            vm_module::bind_method<base_class>(vm, base_name, "get_size", [](auto vm, auto self, auto& id) -> int {
                auto size = self -> get_size();
                vm -> push_value(size.x);
                vm -> push_value(size.y);
                return 2;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_active", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> is_active());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_active", [](auto vm, auto self, auto& id) -> int {
                auto clear = vm -> is_bool(2) ? vm -> get_bool(2) : false;
                auto instant = vm -> is_bool(3) ? vm -> get_bool(3) : false;
                base_class::set_active(self, clear, instant);
                vm -> push_value(true);
                return 1;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Rendertarget : vm_module {};
}
#endif