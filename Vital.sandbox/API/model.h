/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: model.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Model APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/model.h>


////////////////////////
// Vital: API: Model //
////////////////////////

namespace Vital::Sandbox::API {
    struct Model : vm_module {
        inline static const std::string base_name = "model";
        using base_class = Engine::Model;

        static void bind(Machine* vm) {
            vm_module::register_type<Model>(vm, base_name);

            API::bind(vm, {base_name}, "is_loaded", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto name = vm -> get_string(1);
                vm -> push_value(base_class::is_model_loaded(name));
                return 1;
            });

            API::bind(vm, {base_name}, "load", [](auto vm) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_string(1)) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto name = vm -> get_string(1);
                auto path = vm -> get_string(2);
                vm -> push_value(base_class::load(name, path));
                return 1;
            });

            API::bind(vm, {base_name}, "unload", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto name = vm -> get_string(1);
                vm -> push_value(base_class::unload(name));
                return 1;
            });

            API::bind(vm, {base_name}, "create", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto name = vm -> get_string(1);
                auto object = base_class::create(name);
                vm -> create_object(base_name, object);
                return 1;
            });

            #if defined(Vital_SDK_Client)
            #else
            API::bind(vm, {base_name}, "create_synced", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto name = vm -> get_string(1);
                int authority = vm -> is_number(2) ? vm -> get_int(2) : 1;
                base_class::create_synced(name, authority);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_synced", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto name = vm -> get_string(1);
                auto object = base_class::get_synced(name);
                if (!object) { vm -> push_nil(); return 1; }
                vm -> create_object(base_name, object);
                return 1;
            });
            #endif
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<base_class>(vm, base_name, "destroy", [](auto vm, auto self) -> int {
                self -> destroy();
                vm_module::release_userdata(vm, 1);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_component_visible", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto component = vm -> get_string(2);
                vm -> push_value(self -> is_component_visible(component));
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_material_visible", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 3) || (!vm -> is_string(2)) || (!vm -> is_string(3))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto component = vm -> get_string(2);
                auto material = vm -> get_string(3);
                vm -> push_value(self -> is_material_visible(component, material));
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_material_feature", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 4) || (!vm -> is_string(2)) || (!vm -> is_string(3)) || (!vm -> is_number(4))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto component = vm -> get_string(2);
                auto material = vm -> get_string(3);
                auto feature = vm -> get_int(4);
                vm -> push_value(self -> is_material_feature(component, material, feature));
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_material_flag", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 4) || (!vm -> is_string(2)) || (!vm -> is_string(3)) || (!vm -> is_number(4))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto component = vm -> get_string(2);
                auto material = vm -> get_string(3);
                auto flag = vm -> get_int(4);
                vm -> push_value(self -> is_material_flag(component, material, flag));
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_animation_playing", [](auto vm, auto self) -> int {
                vm -> push_value(self -> is_animation_playing());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_synced", [](auto vm, auto self) -> int {
                vm -> push_value(self -> is_synced());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_position", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_vector3(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto position = vm -> get_vector3(2);
                self -> set_position(position);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_rotation", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_vector3(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto rotation = vm -> get_vector3(2);
                self -> set_rotation(rotation);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_component_visible", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 3) || (!vm -> is_string(2)) || (!vm -> is_bool(3))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto component = vm -> get_string(2);
                auto state = vm -> get_bool(3);
                self -> set_component_visible(component, state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_material_visible", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 4) || (!vm -> is_string(2)) || (!vm -> is_string(3)) || (!vm -> is_bool(4))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto component = vm -> get_string(2);
                auto material = vm -> get_string(3);
                auto state = vm -> get_bool(4);
                vm -> push_value(self -> set_material_visible(component, material, state));
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_material_feature", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 5) || (!vm -> is_string(2)) || (!vm -> is_string(3)) || (!vm -> is_number(4)) || (!vm -> is_bool(5))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto component = vm -> get_string(2);
                auto material = vm -> get_string(3);
                auto feature = vm -> get_int(4);
                auto state = vm -> get_bool(5);
                vm -> push_value(self -> set_material_feature(component, material, feature, state));
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_material_flag", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 5) || (!vm -> is_string(2)) || (!vm -> is_string(3)) || (!vm -> is_number(4)) || (!vm -> is_bool(5))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto component = vm -> get_string(2);
                auto material = vm -> get_string(3);
                auto flag = vm -> get_int(4);
                auto state = vm -> get_bool(5);
                vm -> push_value(self -> set_material_flag(component, material, flag, state));
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_blendshape_value", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 4) || (!vm -> is_string(2)) || (!vm -> is_string(3)) || (!vm -> is_number(4))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto component = vm -> get_string(2);
                auto blendshape = vm -> get_string(3);
                auto value = vm -> get_float(4);
                self -> set_blendshape_value(component, blendshape, value);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "set_animation_speed", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_number(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto speed = vm -> get_float(2);
                self -> set_animation_speed(speed);
                vm -> push_value(true);
                return 1;
            });

            #if defined(Vital_SDK_Client)
            #else
            vm_module::bind_method<base_class>(vm, base_name, "set_sync_authority", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_number(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto peer_id = vm -> get_int(2);
                self -> set_sync_authority(peer_id);
                vm -> push_value(true);
                return 1;
            });
            #endif

            vm_module::bind_method<base_class>(vm, base_name, "get_model_name", [](auto vm, auto self) -> int {
                vm -> push_value(self -> get_model_name());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_position", [](auto vm, auto self) -> int {
                vm -> push_value(self -> get_position());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_rotation", [](auto vm, auto self) -> int {
                vm -> push_value(self -> get_rotation());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_components", [](auto vm, auto self) -> int {
                auto list = self -> get_components();
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_value(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_materials", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto component = vm -> get_string(2);
                auto list = self -> get_materials(component);
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_value(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_blendshapes", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto component = vm -> get_string(2);
                auto list = self -> get_blendshapes(component);
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_value(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_bones", [](auto vm, auto self) -> int {
                auto list = self -> get_bones();
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_value(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_animations", [](auto vm, auto self) -> int {
                auto list = self -> get_animations();
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_value(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_blendshape_value", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 3) || (!vm -> is_string(2)) || (!vm -> is_string(3))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto component = vm -> get_string(2);
                auto blendshape = vm -> get_string(3);
                vm -> push_value(self -> get_blendshape_value(component, blendshape));
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_bone_position", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto bone = vm -> get_string(2);
                vm -> push_value(self -> get_bone_position(bone));
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_current_animation", [](auto vm, auto self) -> int {
                vm -> push_value(self -> get_current_animation());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_animation_speed", [](auto vm, auto self) -> int {
                vm -> push_value(self -> get_animation_speed());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "get_sync_authority", [](auto vm, auto self) -> int {
                vm -> push_value(self -> get_sync_authority());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "play_animation", [](auto vm, auto self) -> int {
                if ((vm -> get_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto name = vm -> get_string(2);
                auto loop = vm -> is_bool(3) ? vm -> get_bool(3) : true;
                auto speed = vm -> is_number(4) ? vm -> get_float(4) : 1.0f;
                vm -> push_value(self -> play_animation(name, loop, speed));
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "stop_animation", [](auto vm, auto self) -> int {
                self -> stop_animation();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "pause_animation", [](auto vm, auto self) -> int {
                self -> pause_animation();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "resume_animation", [](auto vm, auto self) -> int {
                self -> resume_animation();
                vm -> push_value(true);
                return 1;
            });
        }
    };
}