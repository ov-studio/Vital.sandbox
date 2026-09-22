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
#include <Vital.sandbox/API/utility/file.h>
#include <Vital.sandbox/API/base/node_3d.h>
#include <Vital.sandbox/API/base/syncable.h>


////////////////////////
// Vital: API: Model //
////////////////////////

namespace Vital::Sandbox::API {
    struct Model : vm_module {
        inline static const std::vector<std::string> base_scope = {"core", "model"};
        inline static constexpr bool has_remote = true;
        using base_class = Vital::Engine::Model;

        struct Instance : vm_instance<Instance> {
            using Owner = Model;
            base_class* model = nullptr;

            auto get_node() { 
                return model; 
            }

            bool is_alive() const {
                return model ? true : false;
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

        inline static std::mutex scope_mutex;
        inline static std::unordered_map<std::string, std::string> model_scope;

        static std::string own_name(const std::string& resource, const std::string& name) {
            return fmt::format(":{}/{}", resource, name);
        }

        static std::string ref_name(const std::string& resource, const std::string& name) {
            if (!name.empty() && name.front() == ':') return name;
            return fmt::format(":{}/{}", resource, name);
        }

        static void init(Machine* vm) {
            static Tool::Event::Handle spawned_binding;
            spawned_binding.bind("entity:spawned", [](Tool::Stack args) {
                if (args.array.size() < 2) return;
                if (!args.array[0].is_raw_ptr<base_class>()) return;
                auto entity = args.array[0].as_raw_ptr<base_class>();
                if (!args.array[1].is<bool>()) return;
                bool remote = args.array[1].as<bool>();
                if (Instance::find_by_ptr(entity)) return;

                const godot::ObjectID oid(entity -> get_instance_id());
                Vital::Engine::Core::get_singleton() -> enqueue([oid, remote]() {
                    godot::Object* obj = godot::ObjectDB::get_instance(oid);
                    if (!obj) return;
                    auto model = godot::Object::cast_to<base_class>(obj);
                    if (!model) return;
                    if (Instance::find_by_ptr(model)) return;
                    
                    auto instance = Instance::init(nullptr, remote);
                    instance -> model = model;
                    instance -> store();
                });
            });

            static Tool::Event::Handle unspawned_binding;
            unspawned_binding.bind("entity:unspawned", [](Tool::Stack args) {
                if (args.array.size() < 1) return;
                if (!args.array[0].is_raw_ptr<base_class>()) return;
                
                auto entity = args.array[0].as_raw_ptr<base_class>();
                Instance::destroy_by_ptr(entity, [](std::shared_ptr<Instance> instance) {
                    instance -> model = nullptr;
                });
            });
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Model>(vm);

            API::bind(vm, base_scope, "load", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, path)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_string);

                const std::string name = vm -> get_string(1);
                std::string path = vm -> get_string(2);
                const std::string resource = Manager::Resource::get_resource_from_vm(vm);
                const std::string key = own_name(resource, name);
                const std::string base = API::File::assert_file(vm, path);
                bool result = base_class::load(key, base, path);
                if (result) {
                    std::lock_guard<std::mutex> lock(scope_mutex);
                    model_scope[key] = resource;
                }
                vm -> push_value(result);
                return 1;
            });

            API::bind(vm, base_scope, "unload", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name)")
                    .require(1, &Machine::is_string);

                const std::string name = vm -> get_string(1);
                const std::string resource = Manager::Resource::get_resource_from_vm(vm);
                const std::string key = own_name(resource, name);
                bool result = base_class::unload(key);
                if (result) {
                    std::lock_guard<std::mutex> lock(scope_mutex);
                    model_scope.erase(key);
                }
                vm -> push_value(result);
                return 1;
            });

            API::bind(vm, base_scope, "is_loaded", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name)")
                    .require(1, &Machine::is_string);

                vm -> push_value(base_class::is_model_loaded(ref_name(Manager::Resource::get_resource_from_vm(vm), vm -> get_string(1))));
                return 1;
            });

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, authority = 1)")
                    .require(1, &Machine::is_string)
                    .optional(2, &Machine::is_number);

                auto name = vm -> get_string(1);
                int authority = vm -> is_number(2) ? vm -> get_int(2) : 1;
                const std::string key = ref_name(Manager::Resource::get_resource_from_vm(vm), name);
                auto instance = Instance::init(vm);
                instance -> model = base_class::create(key, authority);
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Node_3D::methods<Instance, Node_3D::Type::Spatial>(vm);
            API::Syncable::methods<Instance>(vm);

            vm_module::bind_method<Instance>(vm, "is_component_visible", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component)", true)
                    .require(2, &Machine::is_string);

                auto component = vm -> get_string(2);
                vm -> push_value(self -> model -> is_component_visible(component));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_material_visible", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material)", true)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string);

                auto component = vm -> get_string(2);
                auto material = vm -> get_string(3);
                vm -> push_value(self -> model -> is_material_visible(component, material));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_material_feature", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material, feature)", true)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_number);

                auto component = vm -> get_string(2);
                auto material = vm -> get_string(3);
                auto feature = vm -> get_int(4);
                vm -> push_value(self -> model -> is_material_feature(component, material, feature));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_material_flag", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material, flag)", true)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_number);

                auto component = vm -> get_string(2);
                auto material = vm -> get_string(3);
                auto flag = vm -> get_int(4);
                vm -> push_value(self -> model -> is_material_flag(component, material, flag));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_animation_playing", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> is_animation_playing());
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
                vm_args(vm, id, "(component)", true)
                    .require(2, &Machine::is_string);

                auto component = vm -> get_string(2);
                auto list = self -> model -> get_materials(component);
                vm -> create_table();
                for (int i = 0; i < (int)list.size(); i++) {
                    vm -> push_value(list[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_blendshapes", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component)", true)
                    .require(2, &Machine::is_string);

                auto component = vm -> get_string(2);
                auto list = self -> model -> get_blendshapes(component);
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
                vm_args(vm, id, "(component, blendshape)", true)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string);

                auto component = vm -> get_string(2);
                auto blendshape = vm -> get_string(3);
                vm -> push_value(self -> model -> get_blendshape_value(component, blendshape));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_bone_position", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(bone)", true)
                    .require(2, &Machine::is_string);

                auto bone = vm -> get_string(2);
                vm -> push_value(self -> model -> get_bone_position(bone));
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

            vm_module::bind_method<Instance>(vm, "set_component_visible", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, state)", true)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_bool);

                auto component = vm -> get_string(2);
                auto state = vm -> get_bool(3);
                self -> model -> set_component_visible(component, state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_material_visible", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material, state)", true)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_bool);

                auto component = vm -> get_string(2);
                auto material = vm -> get_string(3);
                auto state = vm -> get_bool(4);
                vm -> push_value(self -> model -> set_material_visible(component, material, state));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_material_feature", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material, feature, state)", true)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_number)
                    .require(5, &Machine::is_bool);

                auto component = vm -> get_string(2);
                auto material = vm -> get_string(3);
                auto feature = vm -> get_int(4);
                auto state = vm -> get_bool(5);
                vm -> push_value(self -> model -> set_material_feature(component, material, feature, state));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_material_flag", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, material, flag, state)", true)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_number)
                    .require(5, &Machine::is_bool);

                auto component = vm -> get_string(2);
                auto material = vm -> get_string(3);
                auto flag = vm -> get_int(4);
                auto state = vm -> get_bool(5);
                vm -> push_value(self -> model -> set_material_flag(component, material, flag, state));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_blendshape_value", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(component, blendshape, value)", true)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_number);

                auto component = vm -> get_string(2);
                auto blendshape = vm -> get_string(3);
                auto value = vm -> get_float(4);
                self -> model -> set_blendshape_value(component, blendshape, value);
                vm -> push_value(true);
                return 1;
            });

            API::Syncable::bind_method<Instance>(vm, "set_animation_speed", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(speed)", true)
                    .require(2, &Machine::is_number);

                auto speed = vm -> get_float(2);
                self -> model -> set_animation_speed(speed);
                vm -> push_value(true);
                return 1;
            });

            #if !defined(VSDK_Client)
            vm_module::bind_method<Instance>(vm, "set_syncer", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(peer_id)", true)
                    .require(2, &Machine::is_number);

                int peer_id = (int)(vm -> get_int(2));
                self -> model -> set_syncer(peer_id);
                vm -> push_value(true);
                return 1;
            });
            #endif

            API::Syncable::bind_method<Instance>(vm, "play_animation_layer", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(layer, name, loop = true, speed = 1, weight = 1, blend_time = 0.25, sync = true)", true)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_string)
                    .optional(4, &Machine::is_bool)
                    .optional(5, &Machine::is_number)
                    .optional(6, &Machine::is_number)
                    .optional(7, &Machine::is_number)
                    .optional(8, &Machine::is_bool);

                auto layer = vm -> get_int(2);
                auto name = vm -> get_string(3);
                auto loop = vm -> is_bool(4) ? vm -> get_bool(4) : true;
                auto speed = vm -> is_number(5) ? vm -> get_float(5) : 1.0f;
                auto weight = vm -> is_number(6) ? vm -> get_float(6) : 1.0f;
                auto blend_time = vm -> is_number(7) ? vm -> get_float(7) : 0.25f;
                auto sync = vm -> is_bool(8) ? vm -> get_bool(8) : true;
                vm -> push_value(self -> model -> play_animation_layer(layer, name, loop, speed, weight, blend_time, sync));
                return 1;
            });

            API::Syncable::bind_method<Instance>(vm, "stop_animation_layer", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(layer, blend_time = 0.25, sync = true)", true)
                    .require(2, &Machine::is_number)
                    .optional(3, &Machine::is_number)
                    .optional(4, &Machine::is_bool);

                auto layer = vm -> get_int(2);
                auto blend_time = vm -> is_number(3) ? vm -> get_float(3) : 0.25f;
                auto sync = vm -> is_bool(4) ? vm -> get_bool(4) : true;
                self -> model -> stop_animation_layer(layer, blend_time, sync);
                vm -> push_value(true);
                return 1;
            });

            API::Syncable::bind_method<Instance>(vm, "set_animation_layer_weight", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(layer, weight, blend_time = 0, sync = true)", true)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_number)
                    .optional(4, &Machine::is_number)
                    .optional(5, &Machine::is_bool);

                auto layer = vm -> get_int(2);
                auto weight = vm -> get_float(3);
                auto blend_time = vm -> is_number(4) ? vm -> get_float(4) : 0.0f;
                auto sync = vm -> is_bool(5) ? vm -> get_bool(5) : true;
                vm -> push_value(self -> model -> set_animation_layer_weight(layer, weight, blend_time, sync));
                return 1;
            });

            API::Syncable::bind_method<Instance>(vm, "set_animation_layer_speed", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(layer, speed, sync = true)", true)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_number)
                    .optional(4, &Machine::is_bool);

                auto layer = vm -> get_int(2);
                auto speed = vm -> get_float(3);
                auto sync = vm -> is_bool(4) ? vm -> get_bool(4) : true;
                self -> model -> set_animation_layer_speed(layer, speed, sync);
                vm -> push_value(true);
                return 1;
            });

            API::Syncable::bind_method<Instance>(vm, "set_animation_layer_filter", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(layer, enabled, bone_paths?, sync = true)", true)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_bool)
                    .optional(4, &Machine::is_table)
                    .optional(5, &Machine::is_bool);

                int layer = vm -> get_int(2);
                bool enabled = vm -> get_bool(3);
                std::vector<std::string> bones;
                if (vm -> get_count() >= 4 && vm -> is_table(4)) {
                    int n = vm -> get_length(4);
                    bones.reserve(n);
                    for (int i = 1; i <= n; ++i) {
                        vm -> get_table_field(i, 4);
                        if (vm -> is_string(-1)) bones.push_back(vm -> get_string(-1));
                        vm -> pop(1);
                    }
                }
                bool sync = vm -> is_bool(5) ? vm -> get_bool(5) : true;
                vm -> push_value(self -> model -> set_animation_layer_filter(layer, enabled, bones, sync));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_animation_layer_weight", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(layer)", true)
                    .require(2, &Machine::is_number);

                vm -> push_value(self -> model -> get_animation_layer_weight(vm -> get_int(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_animation_layer_speed", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(layer)", true)
                    .require(2, &Machine::is_number);

                vm -> push_value(self -> model -> get_animation_layer_speed(vm -> get_int(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_current_animation_layer", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(layer)", true)
                    .require(2, &Machine::is_number);

                vm -> push_value(self -> model -> get_current_animation_layer(vm -> get_int(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_animation_layer_playing", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(layer)", true)
                    .require(2, &Machine::is_number);

                vm -> push_value(self -> model -> is_animation_layer_playing(vm -> get_int(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_animation_layer_count", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> model -> get_animation_layer_count());
                return 1;
            });
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
            {
                std::lock_guard<std::mutex> lock(scope_mutex);
                std::vector<std::string> to_unload;
                for (const auto& [key, resource_env] : model_scope) {
                    if (resource_env == env) to_unload.push_back(key);
                }
                for (const auto& key : to_unload) {
                    model_scope.erase(key);
                    try { base_class::unload(key); }
                    catch (...) {}
                }
            }
        }
    };
}