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
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/font.h>
#include <Vital.sandbox/API/utility/file.h>


///////////////////////
// Vital: API: Font //
///////////////////////

namespace Vital::Sandbox::API {
    struct Font : vm_module {
        inline static const std::vector<std::string> base_scope = {"core", "font"};
        using base_class = Vital::Engine::Font;

        struct Instance : vm_instance<Instance> {
            using Owner = Font;
            base_class* font = nullptr;

            bool is_alive() const { 
                return font ? true : false;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> font) {
                    instance -> font -> destroy();
                    instance -> font = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static void bind(Machine* vm) {
            vm_module::register_type<Font>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path)")
                    .require(1, &Machine::is_string);
            
                auto path = vm -> get_string(1);
                auto base = API::File::assert_file(vm, path);
                auto instance = Instance::init(vm);
                instance -> font = base_class::create(base, path);
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "get_antialiasing", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> font -> get_antialiasing());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_oversampling", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> font -> get_oversampling());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_antialiasing", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)")
                    .require(2, &Machine::is_bool);

                self -> font -> set_antialiasing(vm -> get_bool(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_oversampling", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(value)")
                    .require(2, &Machine::is_number);

                self -> font -> set_oversampling(static_cast<float>(vm -> get_float(2)));
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
    struct Font : vm_module {};
}
#endif