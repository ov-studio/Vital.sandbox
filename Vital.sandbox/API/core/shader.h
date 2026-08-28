/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: shader.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 22/08/2026
     Desc: Shader APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/shader.h>
#include <Vital.sandbox/Engine/public/model.h>
#include <Vital.sandbox/Engine/public/rendertarget.h>
#include <Vital.sandbox/API/core/image.h>
#include <Vital.sandbox/API/utility/file.h>


/////////////////////////
// Vital: API: Shader //
/////////////////////////

namespace Vital::Sandbox::API {
    struct Shader : vm_module {
        inline static const std::vector<std::string> base_scope = {"core", "shader"};
        using base_class = Vital::Engine::Shader;

        struct Instance : vm_instance<Instance> {
            using Owner = Shader;
            base_class* shader = nullptr;

            bool is_alive() const {
                return shader ? true : false;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> shader) {
                    instance -> shader -> destroy();
                    instance -> shader = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static void bind(Machine* vm) {
            vm_module::register_type<Shader>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(path, mode)")
                    .require(1, &Machine::is_string)
                    .require_enum(2, base_class::mode_registry);

                auto path = vm -> get_string(1);
                auto base = API::File::assert_file(vm, path);
                auto mode = static_cast<base_class::Mode>(vm -> get_int(2));
                auto instance = Instance::init(vm);
                instance -> shader = base_class::create(base, path, mode);
                instance -> store(true);
                return 1;
            });

            API::bind(vm, base_scope, "create_from_raw", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(raw, mode)")
                    .require(1, &Machine::is_string)
                    .require_enum(2, base_class::mode_registry);

                auto raw = vm -> get_string(1);
                auto mode = static_cast<base_class::Mode>(vm -> get_int(2));
                auto instance = Instance::init(vm);
                instance -> shader = base_class::create_from_raw(raw, mode);
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "get_mode", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> shader -> get_mode());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_code", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> shader -> get_code());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_param", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name, value)", true)
                    .require(2, &Machine::is_string)
                    .require(3, [](Machine* vm, int idx) {
                        return vm -> is_bool(idx)
                            || vm -> is_number(idx)
                            || vm -> is_vector2(idx) 
                            || vm -> is_vector3(idx);
                    });

                auto name = vm -> get_string(2);
                godot::Variant value;
                if (vm -> is_number(3))  value = vm -> get_float(3);
                else if (vm -> is_bool(3))    value = vm -> get_bool(3);
                else if (vm -> is_vector2(3)) value = vm -> get_vector2(3);
                else if (vm -> is_vector3(3)) value = vm -> get_vector3(3);
                vm -> push_value(self -> shader -> set_param(name, value));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_param_color", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name, color)", true)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_color);

                auto name = vm -> get_string(2);
                auto color = vm -> get_color(3);
                vm -> push_value(self -> shader -> set_param(name, godot::Variant(color)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_param_texture", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name, texture)", true)
                    .require(2, &Machine::is_string)
                    .require(3, [](Machine* vm, int idx) { return vm_module::is_userdata<API::Image::Instance>(vm, idx); });

                auto name = vm -> get_string(2);
                auto instance = vm_module::get_userdata_object<API::Image::Instance>(vm, 3);
                auto ref = instance -> texture -> get_canvas_texture();
                vm -> push_value(self -> shader -> set_param_texture(name, ref));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_param_rt", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name, rendertarget)", true)
                    .require(2, &Machine::is_string)
                    .require(3, [](Machine* vm, int idx) { return vm_module::is_userdata<API::Rendertarget::Instance>(vm, idx); });

                auto name = vm -> get_string(2);
                auto instance = vm_module::get_userdata_object<API::Rendertarget::Instance>(vm, 3);
                auto vp_tex = instance -> rendertarget -> get_texture();
                vm -> push_value(self -> shader -> set_param_viewport_texture(name, vp_tex));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "apply_to_model", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(model)", true)
                    .require(2, [](Machine* vm, int idx) { return vm_module::is_userdata<Vital::Sandbox::API::Model::Instance>(vm, idx); });

                using ModelInstance = Vital::Sandbox::API::Model::Instance;
                auto instance = vm_module::get_userdata_object<ModelInstance>(vm, 2);
                vm -> push_value(self -> shader -> apply_to_node(instance -> model));
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "mode", base_class::mode_registry);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Shader : vm_module {};
}
#endif
