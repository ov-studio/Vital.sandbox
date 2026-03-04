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
        using base_class = Vital::Godot::Model;

        static void bind(Machine* vm) {
            vm_module::register_type<Model>(vm, base_name);

            API::bind(vm, {base_name}, "is_loaded", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                vm -> push_bool(Vital::Godot::Model::is_model_loaded(vm -> get_string(1)));
                return 1;
            });

            API::bind(vm, {base_name}, "load", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(1)) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                vm -> push_bool(Vital::Godot::Model::load_model(vm -> get_string(1), vm -> get_string(2)));
                return 1;
            });

            API::bind(vm, {base_name}, "unload", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                vm -> push_bool(Vital::Godot::Model::unload_model(vm -> get_string(1)));
                return 1;
            });

            API::bind(vm, {base_name}, "create", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto* object = Vital::Godot::Model::create_object(vm -> get_string(1));
                vm -> create_object(base_name, object);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<base_class>(vm, base_name, "is_component_visible", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                vm -> push_bool(self -> is_component_visible(vm -> get_string(2)));
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_animation_playing", [](auto vm, auto self) -> int {
                vm -> push_bool(self -> is_animation_playing());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_position", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 4) || (!vm -> is_number(2)) || (!vm -> is_number(3)) || (!vm -> is_number(4))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                self -> set_position({vm -> get_float(2), vm -> get_float(3), vm -> get_float(4)});
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_rotation", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 4) || (!vm -> is_number(2)) || (!vm -> is_number(3)) || (!vm -> is_number(4))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                self -> set_rotation({vm -> get_float(2), vm -> get_float(3), vm -> get_float(4)});
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_component_visible", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 3) || (!vm -> is_string(2)) || (!vm -> is_bool(3))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                self -> set_component_visible(vm -> get_string(2), vm -> get_bool(3));
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_animation_speed", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_number(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                self -> set_animation_speed(vm -> get_float(2));
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_blend_shape_value", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 4) || (!vm -> is_string(2)) || (!vm -> is_string(3)) || (!vm -> is_number(4))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                self -> set_blend_shape_value(vm -> get_string(2), vm -> get_string(3), vm -> get_float(4));
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_model_name", [](auto vm, auto self) -> int {
                vm -> push_string(self -> get_model_name());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_position", [](auto vm, auto self) -> int {
                auto p = self -> get_position();
                vm -> push_number(p.x);
                vm -> push_number(p.y);
                vm -> push_number(p.z);
                return 3;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_rotation", [](auto vm, auto self) -> int {
                auto r = self -> get_rotation();
                vm -> push_number(r.x);
                vm -> push_number(r.y);
                vm -> push_number(r.z);
                return 3;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_components", [](auto vm, auto self) -> int {
                auto list = self -> get_components();
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_string(list[i]); vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_animations", [](auto vm, auto self) -> int {
                auto list = self -> get_animations();
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_string(list[i]); vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_blend_shapes", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto list = self -> get_blend_shapes(vm -> get_string(2));
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_string(list[i]); vm -> set_table_field(i + 1, -2);
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
                vm -> push_number(self -> get_blend_shape_value(vm -> get_string(2), vm -> get_string(3)));
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "play_animation", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                bool loop   = vm -> is_bool(3)   ? vm -> get_bool(3)  : true;
                float speed = vm -> is_number(4) ? vm -> get_float(4) : 1.0f;
                vm -> push_bool(self -> play_animation(vm -> get_string(2), loop, speed));
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

            vm_module::bind_method<base_class>(vm, base_name, "destroy", [](auto vm, auto self) -> int {
                self -> queue_free();
                void** ud = static_cast<void**>(lua_touserdata(vm -> get_state(), 1));
                if (ud) *ud = nullptr;
                vm -> push_bool(true);
                return 1;
            });
        }
    };
}