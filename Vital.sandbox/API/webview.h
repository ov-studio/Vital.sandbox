/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: webview.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Webview APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/webview.h>


//////////////////////////
// Vital: API: Webview //
//////////////////////////

namespace Vital::Sandbox::API {
    struct Webview : vm_module {
        inline static const std::string base_name = "webview";
        using base_class = Engine::Webview;

        static std::string handler_key(void* ptr) {
            return "webview_message_handler_" + std::to_string(reinterpret_cast<uintptr_t>(ptr));
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Webview>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm) -> int {
                auto object = base_class::create();
                vm -> create_object(base_name, object);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<base_class>(vm, base_name, "destroy", [](auto vm, auto self) -> int {
                auto key = handler_key(self);
                if (vm -> is_reference(key)) vm -> del_reference(key);
                self -> destroy();
                vm_module::release_userdata(vm, 1);
                vm -> push_value(true);
                return 1;
            });
            
            vm_module::bind_method<base_class>(vm, base_name, "is_visible", [](auto vm, auto self) -> int {
                vm -> push_value(self -> is_visible());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_fullscreen", [](auto vm, auto self) -> int {
                vm -> push_value(self -> is_fullscreen());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_transparent", [](auto vm, auto self) -> int {
                vm -> push_value(self -> is_transparent());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_autoplay", [](auto vm, auto self) -> int {
                vm -> push_value(self -> is_autoplay());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_zoomable", [](auto vm, auto self) -> int {
                vm -> push_value(self -> is_zoomable());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_devtools_visible", [](auto vm, auto self) -> int {
                vm -> push_value(self -> is_devtools_visible());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_position", [](auto vm, auto self) -> int {
                vm -> push_value(self -> get_position());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_size", [](auto vm, auto self) -> int {
                vm -> push_value(self -> get_size());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_visible", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_bool(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(2);
                self -> set_visible(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_fullscreen", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_bool(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(2);
                self -> set_fullscreen(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_transparent", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_bool(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(2);
                self -> set_transparent(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_autoplay", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_bool(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(2);
                self -> set_autoplay(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_zoomable", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_bool(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(2);
                self -> set_zoomable(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_devtools_visible", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_bool(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(2);
                self -> set_devtools_visible(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_position", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_vector2(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto position = vm -> get_vector2(2);
                self -> set_position(position);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_size", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_vector2(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto size = vm -> get_vector2(2);
                self -> set_size(size);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_message_handler", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_function(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto key = handler_key(self);
                vm -> set_reference(key, 2);
                self -> set_message_handler([vm, key](godot::String message) {
                    vm -> get_reference(key, true);
                    vm -> push_value(Tool::to_std_string(message));
                    vm -> pcall(1, 0);
                });
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "load_url", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto url = vm -> get_string(2);
                self -> load_url(url);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "load_html", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto html = vm -> get_string(2);
                self -> load_html(html);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "clear_history", [](auto vm, auto self) -> int {
                self -> clear_history();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "focus", [](auto vm, auto self) -> int {
                self -> focus();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "reload", [](auto vm, auto self) -> int {
                self -> reload();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "zoom", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_number(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(2);
                self -> zoom(value);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "update", [](auto vm, auto self) -> int {
                self -> update();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "eval", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto input = vm -> get_string(2);
                self -> eval(input);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "emit", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto input = vm -> get_string(2);
                self -> emit(input);
                vm -> push_value(true);
                return 1;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Webview : vm_module {};
}
#endif