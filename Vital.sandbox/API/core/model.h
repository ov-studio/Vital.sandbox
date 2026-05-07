/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: model.h
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
        using base_class = Vital::Engine::Model;

        struct Instance : vm_instance<Instance> {
            using Owner = Model;
            base_class* model = nullptr;
        };
        inline static std::mutex mutex;
        inline static std::unordered_map<int, std::shared_ptr<Instance>> buffer;
        inline static std::atomic<int> next_id { 1 };

        static void clean_instance(std::shared_ptr<Instance> instance) {
            if (!Instance::erase(instance)) return;
            if (instance -> model) {
                instance -> model -> destroy();
                instance -> model = nullptr;
            }
            Instance::release(instance);
        }

        static std::shared_ptr<Instance> find_by_ptr(base_class* ptr) {
            if (!ptr) return nullptr;
            std::lock_guard<std::mutex> lock(mutex);
            for (auto& [id, instance] : buffer) {
                if (!instance -> destroyed && instance -> model == ptr) return instance;
            }
            return nullptr;
        }

        static void push_instance(Machine* vm, std::shared_ptr<Instance> instance) {
            instance -> vm -> get_reference(instance -> self_reference(), true);
            instance -> vm -> move(vm, 1);
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Model>(vm, base_name);

            API::bind(vm, {base_name}, "is_loaded", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name)")
                    .require(1, &Machine::is_string);

                vm -> push_value(base_class::is_model_loaded(vm -> get_string(1)));
                return 1;
            });

            API::bind(vm, {base_name}, "load", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, path)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_string);

                vm -> push_value(base_class::load(vm -> get_string(1), vm -> get_string(2)));
                return 1;
            });

            API::bind(vm, {base_name}, "unload", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name)")
                    .require(1, &Machine::is_string);

                vm -> push_value(base_class::unload(vm -> get_string(1)));
                return 1;
            });

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name)")
                    .require(1, &Machine::is_string);

                auto instance = Instance::init(vm);
                instance -> model = base_class::create(vm -> get_string(1));
                Instance::store(instance);
                vm -> create_object(base_name, instance.get());
                instance -> userdata = vm_module::get_userdata_ptr(vm, -1);
                instance -> set_ref(instance -> self_reference(), -1);
                return 1;
            });

            #if defined(Vital_SDK_Client)
            #else
            API::bind(vm, {base_name}, "create_synced", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, authority = 1)")
                    .require(1, &Machine::is_string)
                    .optional(2, &Machine::is_number);

                auto name = vm -> get_string(1);
                int authority = vm -> is_number(2) ? vm -> get_int(2) : 1;
                base_class::create_synced(name, authority);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "get_synced", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name)")
                    .require(1, &Machine::is_string);

                auto ptr = base_class::get_synced(vm -> get_string(1));
                auto instance = find_by_ptr(ptr);
                if (!instance) { vm -> push_nil(); return 1; }
                push_instance(vm, instance);
                return 1;
            });
            #endif
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, base_name, "destroy", [](auto vm, auto self, auto& id) -> int {
                return Instance::destroy(vm);
            });

            vm_module::bind_method<Instance>(vm, base_name, "is_component_visible", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component)")
                    .require(2, &Machine::is_string);

                vm -> push_value(self -> model -> is_component_visible(vm -> get_string(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "is_material_visible", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string);

                vm -> push_value(self -> model -> is_material_visible(vm -> get_string(2), vm -> get_string(3)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "is_material_feature", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material, feature)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_number);

                vm -> push_value(self -> model -> is_material_feature(vm -> get_string(2), vm -> get_string(3), vm -> get_int(4)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "is_material_flag", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material, flag)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_number);

                vm -> push_value(self -> model -> is_material_flag(vm -> get_string(2), vm -> get_string(3), vm -> get_int(4)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "is_animation_playing", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> is_animation_playing());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "is_synced", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> is_synced());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "set_position", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(position)")
                    .require(2, &Machine::is_vector3);

                self -> model -> set_position(vm -> get_vector3(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "set_rotation", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(rotation)")
                    .require(2, &Machine::is_vector3);

                self -> model -> set_rotation(vm -> get_vector3(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "set_component_visible", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, state)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_bool);

                self -> model -> set_component_visible(vm -> get_string(2), vm -> get_bool(3));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "set_material_visible", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material, state)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_bool);

                vm -> push_value(self -> model -> set_material_visible(vm -> get_string(2), vm -> get_string(3), vm -> get_bool(4)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "set_material_feature", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material, feature, state)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_number)
                    .require(5, &Machine::is_bool);

                vm -> push_value(self -> model -> set_material_feature(vm -> get_string(2), vm -> get_string(3), vm -> get_int(4), vm -> get_bool(5)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "set_material_flag", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material, flag, state)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_number)
                    .require(5, &Machine::is_bool);

                vm -> push_value(self -> model -> set_material_flag(vm -> get_string(2), vm -> get_string(3), vm -> get_int(4), vm -> get_bool(5)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "set_blendshape_value", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, blendshape, value)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_number);

                self -> model -> set_blendshape_value(vm -> get_string(2), vm -> get_string(3), vm -> get_float(4));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "set_animation_speed", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(speed)")
                    .require(2, &Machine::is_number);

                self -> model -> set_animation_speed(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            #if defined(Vital_SDK_Client)
            #else
            vm_module::bind_method<Instance>(vm, base_name, "set_sync_authority", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(peer_id)")
                    .require(2, &Machine::is_number);

                self -> model -> set_sync_authority(vm -> get_int(2));
                vm -> push_value(true);
                return 1;
            });
            #endif

            vm_module::bind_method<Instance>(vm, base_name, "get_model_name", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> get_model_name());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "get_position", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> get_position());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "get_rotation", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> get_rotation());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "get_components", [](auto vm, auto self, auto& id) -> int {
                auto list = self -> model -> get_components();
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_value(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "get_materials", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component)")
                    .require(2, &Machine::is_string);

                auto list = self -> model -> get_materials(vm -> get_string(2));
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_value(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "get_blendshapes", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component)")
                    .require(2, &Machine::is_string);

                auto list = self -> model -> get_blendshapes(vm -> get_string(2));
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_value(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "get_bones", [](auto vm, auto self, auto& id) -> int {
                auto list = self -> model -> get_bones();
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_value(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "get_animations", [](auto vm, auto self, auto& id) -> int {
                auto list = self -> model -> get_animations();
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_value(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "get_blendshape_value", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, blendshape)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string);

                vm -> push_value(self -> model -> get_blendshape_value(vm -> get_string(2), vm -> get_string(3)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "get_bone_position", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(bone)")
                    .require(2, &Machine::is_string);

                vm -> push_value(self -> model -> get_bone_position(vm -> get_string(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "get_current_animation", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> get_current_animation());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "get_animation_speed", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> get_animation_speed());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "get_sync_authority", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> get_sync_authority());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "play_animation", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name, loop = true, speed = 1.0)")
                    .require(2, &Machine::is_string)
                    .optional(3, &Machine::is_bool)
                    .optional(4, &Machine::is_number);

                auto loop = vm -> is_bool(3) ? vm -> get_bool(3) : true;
                auto speed = vm -> is_number(4) ? vm -> get_float(4) : 1.0f;
                vm -> push_value(self -> model -> play_animation(vm -> get_string(2), loop, speed));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "stop_animation", [](auto vm, auto self, auto& id) -> int {
                self -> model -> stop_animation();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "pause_animation", [](auto vm, auto self, auto& id) -> int {
                self -> model -> pause_animation();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "resume_animation", [](auto vm, auto self, auto& id) -> int {
                self -> model -> resume_animation();
                vm -> push_value(true);
                return 1;
            });
        }

        static void clean(const std::string& env) {
            vm_module::collect_env<Instance>(mutex, buffer, env, clean_instance);
        }
    };
}