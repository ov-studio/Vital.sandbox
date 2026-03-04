/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: webview.h
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
#include <Vital.extension/Sandbox/index.h>
#include <Vital.extension/Engine/public/webview.h>


///////////////////////////////////
// Vital: Sandbox: API: Webview //
///////////////////////////////////

namespace Vital::Sandbox::API {
    struct Webview : vm_module {
        inline static const std::string base_name = "webview";
        using base_class = Vital::Godot::Webview;

        static void bind(Machine* vm) {
            vm_module::register_type<Webview>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm) -> int {
                auto* object = memnew(Vital::Godot::Webview);
                vm -> create_object(base_name, object);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<base_class>(vm, base_name, "destroy", [](auto vm, auto self) -> int {
                memdelete(self);
                void** ud = static_cast<void**>(lua_touserdata(vm -> get_state(), 1));
                if (ud) *ud = nullptr;
                vm -> push_bool(true);
                return 1;
            });
            
            vm_module::bind_method<base_class>(vm, base_name, "is_visible", [](auto vm, auto self) -> int {
                vm -> push_bool(self -> is_visible());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_fullscreen", [](auto vm, auto self) -> int {
                vm -> push_bool(self -> is_fullscreen());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_transparent", [](auto vm, auto self) -> int {
                vm -> push_bool(self -> is_transparent());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_autoplay", [](auto vm, auto self) -> int {
                vm -> push_bool(self -> is_autoplay());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_zoomable", [](auto vm, auto self) -> int {
                vm -> push_bool(self -> is_zoomable());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_devtools_visible", [](auto vm, auto self) -> int {
                vm -> push_bool(self -> is_devtools_visible());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_position", [](auto vm, auto self) -> int {
                auto p = self -> get_position();
                vm -> push_number(p.x);
                vm -> push_number(p.y);
                return 2;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_size", [](auto vm, auto self) -> int {
                auto s = self -> get_size();
                vm -> push_number(s.x);
                vm -> push_number(s.y);
                return 2;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_visible", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_bool(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(2);
                self -> set_visible(state);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_fullscreen", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_bool(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(2);
                self -> set_fullscreen(state);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_transparent", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_bool(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(2);
                self -> set_transparent(state);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_autoplay", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_bool(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(2);
                self -> set_autoplay(state);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_zoomable", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_bool(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(2);
                self -> set_zoomable(state);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_devtools_visible", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_bool(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_bool(2);
                self -> set_devtools_visible(state);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_position", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 3) || (!vm -> is_number(2)) || (!vm -> is_number(3))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto x = vm -> get_float(2);
                auto y = vm -> get_float(3);
                self -> set_position({x, y});
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_size", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 3) || (!vm -> is_number(2)) || (!vm -> is_number(3))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto x = vm -> get_float(2);
                auto y = vm -> get_float(3);
                self -> set_size({x, y});
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_message_handler", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_function(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                vm -> set_reference("webview_message_handler", 2);
                self -> set_message_handler([vm](godot::String message) {
                    vm -> get_reference("webview_message_handler", true);
                    vm -> push_string(to_std_string(message));
                    vm -> pcall(1, 0);
                });
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "load_url", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto url = vm -> get_string(2);
                self -> load_url(url);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "load_html", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto html = vm -> get_string(2);
                self -> load_html(html);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "clear_history", [](auto vm, auto self) -> int {
                self -> clear_history();
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "focus", [](auto vm, auto self) -> int {
                self -> focus();
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "reload", [](auto vm, auto self) -> int {
                self -> reload();
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "zoom", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_number(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto value = vm -> get_float(2);
                self -> zoom(value);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "update", [](auto vm, auto self) -> int {
                self -> update();
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "eval", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto input = vm -> get_string(2);
                self -> eval(input);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "emit", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto input = vm -> get_string(2);
                self -> emit(input);
                vm -> push_bool(true);
                return 1;
            });
        }
    };
}
#endif