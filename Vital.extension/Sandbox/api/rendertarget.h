/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: rendertarget.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: RenderTarget APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/index.h>
#include <Vital.extension/Engine/public/rendertarget.h>


////////////////////////////////////////
// Vital: Sandbox: API: RenderTarget //
////////////////////////////////////////

namespace Vital::Sandbox::API {
    struct RenderTarget : vm_module {
        inline static const std::string base_name = "rendertarget";
        using base_class = Vital::Engine::RenderTarget;

        static void bind(Machine* vm) {
            vm_module::register_type<RenderTarget>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_number(1)) || (!vm -> is_number(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto width = vm -> get_int(1);
                auto height = vm -> get_int(2);
                auto transparent = vm -> is_bool(3) ? vm -> get_bool(3) : false;
                auto* object = Vital::Engine::RenderTarget::create(width, height, transparent);
                vm -> create_object(base_name, object);
                return 1;
            });

            API::bind(vm, {base_name}, "set_target", [](auto vm) -> int {
                Vital::Engine::RenderTarget* rt = nullptr;
                if ((vm -> get_arg_count() >= 1) && vm -> is_userdata(1)) {
                    rt = static_cast<base_class*>(vm -> get_userdata(1));
                }
                auto clear = vm -> is_bool(2) ? vm -> get_bool(2) : false;
                auto instant = vm -> is_bool(3) ? vm -> get_bool(3) : false;
                Vital::Engine::RenderTarget::set_target(rt, clear, instant);
                vm -> push_bool(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_target", [](auto vm) -> int {
                auto* object = Vital::Engine::RenderTarget::get_target();
                if (!object) vm -> push_bool(false);
                else vm -> create_object(base_name, object);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<base_class>(vm, base_name, "destroy", [](auto vm, auto self) -> int {
                self -> destroy();
                void** ud = static_cast<void**>(lua_touserdata(vm -> get_state(), 1));
                if (ud) *ud = nullptr;
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_size", [](auto vm, auto self) -> int {
                auto size = self -> get_size();
                vm -> push_number(size.x);
                vm -> push_number(size.y);
                return 2;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_target", [](auto vm, auto self) -> int {
                vm -> push_bool(Vital::Engine::RenderTarget::is_target(self));
                return 1;
            });
        
            vm_module::bind_method<base_class>(vm, base_name, "set_target", [](auto vm, auto self) -> int {
                auto clear = vm -> is_bool(2) ? vm -> get_bool(2) : false;
                auto instant = vm -> is_bool(3) ? vm -> get_bool(3) : false;
                Vital::Engine::RenderTarget::set_target(self, clear, instant);
                vm -> push_bool(true);
                return 1;
            });
        }
    };
}