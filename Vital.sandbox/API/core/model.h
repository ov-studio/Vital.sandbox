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
#include <Vital.sandbox/API/utility/promise.h>


////////////////////////
// Vital: API: Model //
////////////////////////

namespace Vital::Sandbox::API {
    struct Model : vm_module {
        inline static const std::string base_name = "model";
        inline static constexpr bool has_remote = true;
        inline static constexpr bool has_streaming = true;
        using base_class = Vital::Engine::Model;

        struct Instance : vm_instance<Instance> {
            using Owner = Model;
            base_class* model = nullptr;

            bool is_alive() const {
                return model ? true : false;
            }

            bool is_streamed() const {
                return model ? model -> is_streamed() : false;
            }

            bool is_remote() const {
                return model ? model -> is_remote() : false;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> model) {
                    instance -> model -> destroy();
                    instance -> model = nullptr;
                }
                instance -> release();
            }

        };
        inline static vm_registry<Instance> registry;

        // Tracks which resource loaded which model name so we can auto-unload on resource stop.
        // model_name → resource_env_name
        inline static std::mutex scope_mutex;
        inline static std::unordered_map<std::string, std::string> model_scope;

        static void bind(Machine* vm) {
            vm_module::register_type<Model>(vm);

            base_class::on_spawned_callback = [vm](base_class* spawned, bool remote) {
                {
                    std::lock_guard<std::mutex> lock(registry.mutex);
                    for (auto& [id, instance] : registry.buffer) {
                        if (instance -> model == spawned) return;
                    }
                }
                auto instance = Instance::init(nullptr, remote);
                instance -> model = spawned;
                instance -> store();
            };

            base_class::on_destroyed_callback = [](base_class* dying) {
                std::lock_guard<std::mutex> lock(registry.mutex);
                for (auto it = registry.buffer.begin(); it != registry.buffer.end();) {
                    auto& instance = it -> second;
                    if (instance -> model != dying) { ++it; continue; }
                    ++it;
                    Instance::erase_unlocked(instance);
                    instance -> model = nullptr;
                    Instance::release(instance);
                }
            };

            API::bind(vm, {base_name}, "load", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, path)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_string);

                const std::string name = vm -> get_string(1);
                const std::string path = vm -> get_string(2);
                const std::string resource = Manager::Resource::get_resource_from_vm(vm);
                // TODO: USE API::FILE::assert_file(vm, path) so it auto detects the asset file and resource base
                // TODO: SHOULD USE SAME FORMAT FOR RESOURCE PATH AS NAME INSTEAD OF "NAME" but when executed inside same resource it should automatically generate :resourcename/ actually the model.h/cpp should use better storing format map for it imo?? or idk its good
                Tool::print("warn", resource);

                bool result = base_class::load(name, path);
                if (result) {
                    std::lock_guard<std::mutex> lock(scope_mutex);
                    model_scope[name] = resource;
                }
                vm -> push_value(result);
                return 1;
            });

            API::bind(vm, {base_name}, "unload", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name)")
                    .require(1, &Machine::is_string);

                const std::string name = vm -> get_string(1);
                bool result = base_class::unload(name);
                if (result) {
                    std::lock_guard<std::mutex> lock(scope_mutex);
                    model_scope.erase(name);
                }
                vm -> push_value(result);
                return 1;
            });

            API::bind(vm, {base_name}, "is_loaded", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name)")
                    .require(1, &Machine::is_string);

                vm -> push_value(base_class::is_model_loaded(vm -> get_string(1)));
                return 1;
            });

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, authority = 1)")
                    .require(1, &Machine::is_string)
                    .optional(2, &Machine::is_number);

                auto name = vm -> get_string(1);
                int authority = vm -> is_number(2) ? vm -> get_int(2) : 1;
                auto instance = Instance::init(vm);
                instance -> model = base_class::create(name, authority);
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "is_component_visible", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component)")
                    .require(2, &Machine::is_string);

                vm -> push_value(self -> model -> is_component_visible(vm -> get_string(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_material_visible", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string);

                vm -> push_value(self -> model -> is_material_visible(vm -> get_string(2), vm -> get_string(3)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_material_feature", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material, feature)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_number);

                vm -> push_value(self -> model -> is_material_feature(vm -> get_string(2), vm -> get_string(3), vm -> get_int(4)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_material_flag", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material, flag)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_number);

                vm -> push_value(self -> model -> is_material_flag(vm -> get_string(2), vm -> get_string(3), vm -> get_int(4)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_animation_playing", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> is_animation_playing());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_model_name", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> get_model_name());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_position", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> get_position());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_rotation", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> get_rotation());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_components", [](auto vm, auto self, auto& id) -> int {
                auto list = self -> model -> get_components();
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_value(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_materials", [](auto vm, auto self, auto& id) -> int {
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

            vm_module::bind_method<Instance>(vm, "get_blendshapes", [](auto vm, auto self, auto& id) -> int {
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

            vm_module::bind_method<Instance>(vm, "get_bones", [](auto vm, auto self, auto& id) -> int {
                auto list = self -> model -> get_bones();
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_value(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_animations", [](auto vm, auto self, auto& id) -> int {
                auto list = self -> model -> get_animations();
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_value(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_blendshape_value", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, blendshape)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string);

                vm -> push_value(self -> model -> get_blendshape_value(vm -> get_string(2), vm -> get_string(3)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_bone_position", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(bone)")
                    .require(2, &Machine::is_string);

                vm -> push_value(self -> model -> get_bone_position(vm -> get_string(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_current_animation", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> get_current_animation());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_animation_speed", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> get_animation_speed());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_sync_authority", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> get_sync_authority());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_position", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(position)")
                    .require(2, &Machine::is_vector3);

                self -> model -> set_position(vm -> get_vector3(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_rotation", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(rotation)")
                    .require(2, &Machine::is_vector3);

                self -> model -> set_rotation(vm -> get_vector3(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_component_visible", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, state)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_bool);

                self -> model -> set_component_visible(vm -> get_string(2), vm -> get_bool(3));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_material_visible", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material, state)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_bool);

                vm -> push_value(self -> model -> set_material_visible(vm -> get_string(2), vm -> get_string(3), vm -> get_bool(4)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_material_feature", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material, feature, state)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_number)
                    .require(5, &Machine::is_bool);

                vm -> push_value(self -> model -> set_material_feature(vm -> get_string(2), vm -> get_string(3), vm -> get_int(4), vm -> get_bool(5)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_material_flag", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material, flag, state)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_number)
                    .require(5, &Machine::is_bool);

                vm -> push_value(self -> model -> set_material_flag(vm -> get_string(2), vm -> get_string(3), vm -> get_int(4), vm -> get_bool(5)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_blendshape_value", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, blendshape, value)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_number);

                self -> model -> set_blendshape_value(vm -> get_string(2), vm -> get_string(3), vm -> get_float(4));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_animation_speed", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(speed)")
                    .require(2, &Machine::is_number);

                self -> model -> set_animation_speed(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            #if !defined(VSDK_Client)
            vm_module::bind_method<Instance>(vm, "set_sync_authority", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(peer_id)")
                    .require(2, &Machine::is_number);

                self -> model -> set_sync_authority(vm -> get_int(2));
                vm -> push_value(true);
                return 1;
            });
            #endif

            vm_module::bind_method<Instance>(vm, "play_animation", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name, loop = true, speed = 1.0)")
                    .require(2, &Machine::is_string)
                    .optional(3, &Machine::is_bool)
                    .optional(4, &Machine::is_number);

                auto loop = vm -> is_bool(3) ? vm -> get_bool(3) : true;
                auto speed = vm -> is_number(4) ? vm -> get_float(4) : 1.0f;
                vm -> push_value(self -> model -> play_animation(vm -> get_string(2), loop, speed));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "stop_animation", [](auto vm, auto self, auto& id) -> int {
                self -> model -> stop_animation();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "pause_animation", [](auto vm, auto self, auto& id) -> int {
                self -> model -> pause_animation();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "resume_animation", [](auto vm, auto self, auto& id) -> int {
                self -> model -> resume_animation();
                vm -> push_value(true);
                return 1;
            });
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);

            // TODO: Unload all model assets that were loaded by this resource env, should below be removed and call Engine::Model::unload_resource_models(name); adapt it from resource.cpp to work accordingly?? 
            {
                std::lock_guard<std::mutex> lock(scope_mutex);
                std::vector<std::string> to_unload;
                for (const auto& [model_name, resource_env] : model_scope) {
                    if (resource_env == env) to_unload.push_back(model_name);
                }
                for (const auto& model_name : to_unload) {
                    model_scope.erase(model_name);
                    try { base_class::unload(model_name); }
                    catch (...) {}
                }
            }
        }
    };
}