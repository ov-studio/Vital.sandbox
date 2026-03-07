/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: api: model.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Model APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/index.h>
#include <Vital.extension/Engine/public/model.h>


/////////////////////////////////
// Vital: Sandbox: API: Model //
/////////////////////////////////

namespace Vital::Sandbox::API {
    struct Model : vm_module {
        inline static const std::string base_name = "model";
        using base_class = Vital::Engine::Model;

        static void bind(Machine* vm) {
            vm_module::register_type<Model>(vm, base_name);

            API::bind(vm, {base_name}, "is_loaded", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto name = vm -> get_string(1);
                vm -> push_bool(Vital::Engine::Model::is_model_loaded(name));
                return 1;
            });

            API::bind(vm, {base_name}, "load", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(1)) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto name = vm -> get_string(1);
                auto path = vm -> get_string(2);
                vm -> push_bool(Vital::Engine::Model::load(name, path));
                return 1;
            });

            API::bind(vm, {base_name}, "unload", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto name = vm -> get_string(1);
                vm -> push_bool(Vital::Engine::Model::unload(name));
                return 1;
            });

            API::bind(vm, {base_name}, "create", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto name = vm -> get_string(1);
                auto* object = Vital::Engine::Model::create(name);
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

            vm_module::bind_method<base_class>(vm, base_name, "is_component_visible", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto name = vm -> get_string(2);
                vm -> push_bool(self -> is_component_visible(name));
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_animation_playing", [](auto vm, auto self) -> int {
                vm -> push_bool(self -> is_animation_playing());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_position", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_vector3(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto position = vm -> get_vector3(2);
                self -> set_position(position);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_rotation", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_vector3(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto rotation = vm -> get_vector3(2);
                self -> set_rotation(rotation);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_component_visible", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 3) || (!vm -> is_string(2)) || (!vm -> is_bool(3))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto name  = vm -> get_string(2);
                auto state = vm -> get_bool(3);
                self -> set_component_visible(name, state);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_animation_speed", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_number(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto speed = vm -> get_float(2);
                self -> set_animation_speed(speed);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_blend_shape_value", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 4) || (!vm -> is_string(2)) || (!vm -> is_string(3)) || (!vm -> is_number(4))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto component   = vm -> get_string(2);
                auto blend_shape = vm -> get_string(3);
                auto value       = vm -> get_float(4);
                self -> set_blend_shape_value(component, blend_shape, value);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_model_name", [](auto vm, auto self) -> int {
                vm -> push_string(self -> get_model_name());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_position", [](auto vm, auto self) -> int {
                auto position = self -> get_position();
                vm -> push_vector3(position);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_rotation", [](auto vm, auto self) -> int {
                auto rotation = self -> get_rotation();
                vm -> push_vector3(rotation);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_components", [](auto vm, auto self) -> int {
                auto list = self -> get_components();
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_string(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_animations", [](auto vm, auto self) -> int {
                auto list = self -> get_animations();
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_string(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_blend_shapes", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto component = vm -> get_string(2);
                auto list = self -> get_blend_shapes(component);
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_string(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_current_animation", [](auto vm, auto self) -> int {
                vm -> push_string(self -> get_current_animation());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_animation_speed", [](auto vm, auto self) -> int {
                vm -> push_number(self -> get_animation_speed());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_blend_shape_value", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 3) || (!vm -> is_string(2)) || (!vm -> is_string(3))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto component   = vm -> get_string(2);
                auto blend_shape = vm -> get_string(3);
                vm -> push_number(self -> get_blend_shape_value(component, blend_shape));
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "play_animation", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto name  = vm -> get_string(2);
                auto loop  = vm -> is_bool(3)   ? vm -> get_bool(3)  : true;
                auto speed = vm -> is_number(4) ? vm -> get_float(4) : 1.0f;
                vm -> push_bool(self -> play_animation(name, loop, speed));
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "stop_animation", [](auto vm, auto self) -> int {
                self -> stop_animation();
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "pause_animation", [](auto vm, auto self) -> int {
                self -> pause_animation();
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "resume_animation", [](auto vm, auto self) -> int {
                self -> resume_animation();
                vm -> push_bool(true);
                return 1;
            });
        }
    };
}