/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: webview.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Webview APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/webview.h>


//////////////////////////
// Vital: API: Webview //
//////////////////////////

namespace Vital::Sandbox::API {
    struct Webview : vm_module {
        inline static const std::vector<std::string> base_scope = {"core", "webview"};
        using base_class = Vital::Engine::Webview;

        struct Instance : vm_instance<Instance> {
            using Owner = Webview;
            base_class* webview = nullptr;
            std::string handler_reference() const { return fmt::format("vm_instance:{}:{}:handler", Owner::base_name, id); }

            bool is_alive() const { 
                return webview ? true : false; 
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> webview) {
                    instance -> webview -> destroy();
                    instance -> webview = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static void bind(Machine* vm) {
            vm_module::register_type<Webview>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                base_class::Options options;
                if (vm -> is_table(1)) {
                    vm -> get_table_field("fullscreen", 1); options.fullscreen = vm -> is_bool(-1) ? vm -> get_bool(-1) : options.fullscreen;
                    vm -> get_table_field("transparent", 1); options.transparent = vm -> is_bool(-1) ? vm -> get_bool(-1) : options.transparent;
                    vm -> get_table_field("incognito", 1); options.incognito = vm -> is_bool(-1) ? vm -> get_bool(-1) : options.incognito;
                    vm -> get_table_field("autoplay", 1); options.autoplay = vm -> is_bool(-1) ? vm -> get_bool(-1) : options.autoplay;
                    vm -> get_table_field("zoomable", 1); options.zoomable = vm -> is_bool(-1) ? vm -> get_bool(-1) : options.zoomable;
                    vm -> pop(5);
                }
                auto instance = Instance::init(vm);
                instance -> webview = base_class::create(options);
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "is_visible", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> webview -> is_visible());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_fullscreen", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> webview -> is_fullscreen());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_transparent", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> webview -> is_transparent());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_incognito", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> webview -> is_incognito());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_autoplay", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> webview -> is_autoplay());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_zoomable", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> webview -> is_zoomable());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_devtools_visible", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> webview -> is_devtools_visible());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_position", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> webview -> get_position());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_size", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> webview -> get_size());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_visible", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(2, &Machine::is_bool);

                self -> webview -> set_visible(vm -> get_bool(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_position", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(position)")
                    .require(2, &Machine::is_vector2);

                self -> webview -> set_position(vm -> get_vector2(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_size", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(size)")
                    .require(2, &Machine::is_vector2);

                self -> webview -> set_size(vm -> get_vector2(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_devtools_visible", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(2, &Machine::is_bool);

                self -> webview -> set_devtools_visible(vm -> get_bool(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_message_handler", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(handler)")
                    .require(2, &Machine::is_function);

                self -> set_reference(self -> handler_reference(), 2);
                auto instance_id = self -> id;
                self -> webview -> set_message_handler([vm, instance_id](godot::String message) {
                    auto instance = Instance::find(instance_id);
                    if (!instance) return;
                    instance -> get_reference(instance -> handler_reference(), true);
                    vm -> push_value(Tool::to_std_string(message));
                    vm -> pcall(1, 0);
                });
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "load_url", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(url)")
                    .require(2, &Machine::is_string);

                self -> webview -> load_url(vm -> get_string(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "load_html", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(html)")
                    .require(2, &Machine::is_string);

                self -> webview -> load_html(vm -> get_string(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "clear_history", [](auto vm, auto self, auto& id) -> int {
                self -> webview -> clear_history();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "focus", [](auto vm, auto self, auto& id) -> int {
                self -> webview -> focus();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "reload", [](auto vm, auto self, auto& id) -> int {
                self -> webview -> reload();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "zoom", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(2, &Machine::is_number);

                self -> webview -> zoom(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "update", [](auto vm, auto self, auto& id) -> int {
                self -> webview -> update();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "eval", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(input)")
                    .require(2, &Machine::is_string);

                self -> webview -> eval(vm -> get_string(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "emit", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(input)")
                    .require(2, &Machine::is_string);

                self -> webview -> emit(vm -> get_string(2));
                vm -> push_value(true);
                return 1;
            });
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Webview : vm_module {};
}
#endif