/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: physics: area.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Area APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/area.h>
#include <Vital.sandbox/API/core/collision_object.h>
#include <Vital.sandbox/API/physics/rigid_body.h>
#include <Vital.sandbox/API/physics/static_body.h>
#include <Vital.sandbox/API/physics/character_body.h>
#include <Vital.sandbox/API/physics/animatable_body.h>


///////////////////////
// Vital: API: Area //
///////////////////////

namespace Vital::Sandbox::API {
    struct Area : vm_module {
        inline static const std::vector<std::string> base_scope = {"physics", "area"};
        using base_class = Vital::Engine::Area;

        inline static const std::vector<std::pair<std::string, base_class::SpaceOverride>> space_override_registry = {
            { "DISABLED",         base_class::SPACE_OVERRIDE_DISABLED         },
            { "COMBINE",          base_class::SPACE_OVERRIDE_COMBINE          },
            { "COMBINE_REPLACE",  base_class::SPACE_OVERRIDE_COMBINE_REPLACE  },
            { "REPLACE",          base_class::SPACE_OVERRIDE_REPLACE          },
            { "REPLACE_COMBINE",  base_class::SPACE_OVERRIDE_REPLACE_COMBINE  }
        };

        struct Instance : vm_instance<Instance> {
            using Owner = Area;
            base_class* body = nullptr;

            auto get_node() {
                return body;
            }

            bool is_alive() const {
                return body ? true : false;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> body) {
                    instance -> body -> destroy();
                    instance -> body = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static std::shared_ptr<Instance> find_by_ptr(base_class* ptr) {
            if (!ptr) return nullptr;
            std::lock_guard<std::mutex> lock(registry.mutex);
            for (auto& [id, instance] : registry.buffer) {
                if (Instance::find_unlocked(instance) && (instance -> get_node() == ptr)) return instance;
            }
            return nullptr;
        }


        static void resolve_entity(std::shared_ptr<Instance>& self, const std::string& signal, godot::Node3D* other) {
            if (auto ptr = godot::Object::cast_to<Vital::Engine::Rigid_Body>(other)) {
                if (auto entity = Rigid_Body::find_by_ptr(ptr)) Manager::Sandbox::get_singleton() -> signal(signal, Tool::StackValue(self), Tool::StackValue(entity));
            } 
            else if (auto ptr = godot::Object::cast_to<Vital::Engine::Static_Body>(other)) {
                if (auto entity = Static_Body::find_by_ptr(ptr)) Manager::Sandbox::get_singleton() -> signal(signal, Tool::StackValue(self), Tool::StackValue(entity));
            } 
            else if (auto ptr = godot::Object::cast_to<Vital::Engine::Character_Body>(other)) {
                if (auto entity = Character_Body::find_by_ptr(ptr)) Manager::Sandbox::get_singleton() -> signal(signal, Tool::StackValue(self), Tool::StackValue(entity));
            } 
            else if (auto ptr = godot::Object::cast_to<Vital::Engine::Animatable_Body>(other)) {
                if (auto entity = Animatable_Body::find_by_ptr(ptr)) Manager::Sandbox::get_singleton() -> signal(signal, Tool::StackValue(self), Tool::StackValue(entity));
            } 
            else if (auto ptr = godot::Object::cast_to<Vital::Engine::Area>(other)) {
                if (auto entity = Area::find_by_ptr(ptr)) Manager::Sandbox::get_singleton() -> signal(signal, Tool::StackValue(self), Tool::StackValue(entity));
            }
        }

        static bool push_entity(Machine* vm, godot::Node3D* node) {
            if (auto ptr = godot::Object::cast_to<Vital::Engine::Rigid_Body>(node)) {
                if (auto instance = Rigid_Body::find_by_ptr(ptr)) { instance -> get_reference(instance -> self_reference(), true, vm); return true; }
            }
            if (auto ptr = godot::Object::cast_to<Vital::Engine::Static_Body>(node)) {
                if (auto instance = Static_Body::find_by_ptr(ptr)) { instance -> get_reference(instance -> self_reference(), true, vm); return true; }
            }
            if (auto ptr = godot::Object::cast_to<Vital::Engine::Character_Body>(node)) {
                if (auto instance = Character_Body::find_by_ptr(ptr)) { instance -> get_reference(instance -> self_reference(), true, vm); return true; }
            }
            if (auto ptr = godot::Object::cast_to<Vital::Engine::Animatable_Body>(node)) {
                if (auto instance = Animatable_Body::find_by_ptr(ptr)) { instance -> get_reference(instance -> self_reference(), true, vm); return true; }
            }
            if (auto ptr = godot::Object::cast_to<Vital::Engine::Area>(node)) {
                if (auto instance = Area::find_by_ptr(ptr)) { instance -> get_reference(instance -> self_reference(), true, vm); return true; }
            }
            return false;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Area>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                auto instance = Instance::init(vm);
                instance -> body = base_class::create();
                std::weak_ptr<Instance> weak = instance;

                auto key = std::to_string(reinterpret_cast<uint64_t>(instance -> body));

                Tool::Event::bind("area:body_entered:"  + key, [weak](Tool::Stack args) mutable {
                    auto self = weak.lock(); if (!self || !Instance::find_unlocked(self) || args.array.size() < 1) return;
                    resolve_entity(self, "area:enter", args.array[0].as<godot::Node3D*>());
                });
                Tool::Event::bind("area:body_exited:"   + key, [weak](Tool::Stack args) mutable {
                    auto self = weak.lock(); if (!self || !Instance::find_unlocked(self) || args.array.size() < 1) return;
                    resolve_entity(self, "area:leave", args.array[0].as<godot::Node3D*>());
                });
                Tool::Event::bind("area:area_entered:"  + key, [weak](Tool::Stack args) mutable {
                    auto self = weak.lock(); if (!self || !Instance::find_unlocked(self) || args.array.size() < 1) return;
                    resolve_entity(self, "area:enter", args.array[0].as<godot::Node3D*>());
                });
                Tool::Event::bind("area:area_exited:"   + key, [weak](Tool::Stack args) mutable {
                    auto self = weak.lock(); if (!self || !Instance::find_unlocked(self) || args.array.size() < 1) return;
                    resolve_entity(self, "area:leave", args.array[0].as<godot::Node3D*>());
                });
                Tool::Event::bind("area:body_shape_entered:"  + key, [weak](Tool::Stack args) mutable {
                    auto self = weak.lock(); if (!self || !Instance::find_unlocked(self) || args.array.size() < 1) return;
                    resolve_entity(self, "area:shape_enter", args.array[0].as<godot::Node3D*>());
                });
                Tool::Event::bind("area:body_shape_exited:"   + key, [weak](Tool::Stack args) mutable {
                    auto self = weak.lock(); if (!self || !Instance::find_unlocked(self) || args.array.size() < 1) return;
                    resolve_entity(self, "area:shape_leave", args.array[0].as<godot::Node3D*>());
                });
                Tool::Event::bind("area:area_shape_entered:"  + key, [weak](Tool::Stack args) mutable {
                    auto self = weak.lock(); if (!self || !Instance::find_unlocked(self) || args.array.size() < 1) return;
                    resolve_entity(self, "area:shape_enter", args.array[0].as<godot::Node3D*>());
                });
                Tool::Event::bind("area:area_shape_exited:"   + key, [weak](Tool::Stack args) mutable {
                    auto self = weak.lock(); if (!self || !Instance::find_unlocked(self) || args.array.size() < 1) return;
                    resolve_entity(self, "area:shape_leave", args.array[0].as<godot::Node3D*>());
                });

                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Collision_Object::methods<Instance, Collision_Object::Type::Area>(vm);

            vm_module::bind_method<Instance>(vm, "get_gravity_space_override_mode", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_gravity_space_override_mode());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_gravity_a_point", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_gravity_a_point());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_gravity_point_center", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_gravity_point_center());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_gravity_direction", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_gravity_direction());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_gravity", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_gravity());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_priority", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_priority());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_monitoring", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_monitoring());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_monitorable", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_monitorable());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "has_overlapping_bodies", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> has_overlapping_bodies());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "has_overlapping_areas", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> has_overlapping_areas());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_overlapping_bodies", [](auto vm, auto self, auto& id) -> int {
                auto bodies = self -> body -> get_overlapping_bodies();
                vm -> create_table();
                int count = 0;
                for (int i = 0; i < bodies.size(); i++) {
                    auto node = godot::Object::cast_to<godot::Node3D>(bodies[i]);
                    if (node && push_entity(vm, node)) vm -> set_table_field(++count, -2);
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_overlapping_areas", [](auto vm, auto self, auto& id) -> int {
                auto areas = self -> body -> get_overlapping_areas();
                vm -> create_table();
                int count = 0;
                for (int i = 0; i < areas.size(); i++) {
                    auto node = godot::Object::cast_to<godot::Node3D>(areas[i]);
                    if (node && push_entity(vm, node)) vm -> set_table_field(++count, -2);
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "overlaps_body", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(body)", true)
                    .require(2, [](Machine* vm, int idx) {
                        return vm_module::is_userdata<Rigid_Body::Instance>(vm, idx) || vm_module::is_userdata<Static_Body::Instance>(vm, idx) ||
                               vm_module::is_userdata<Character_Body::Instance>(vm, idx) || vm_module::is_userdata<Animatable_Body::Instance>(vm, idx);
                    });

                godot::Node* target = nullptr;
                if (vm_module::is_userdata<Rigid_Body::Instance>(vm, 2)) target = vm_module::get_userdata_object<Rigid_Body::Instance>(vm, 2) -> get_node();
                else if (vm_module::is_userdata<Static_Body::Instance>(vm, 2)) target = vm_module::get_userdata_object<Static_Body::Instance>(vm, 2) -> get_node();
                else if (vm_module::is_userdata<Character_Body::Instance>(vm, 2)) target = vm_module::get_userdata_object<Character_Body::Instance>(vm, 2) -> get_node();
                else if (vm_module::is_userdata<Animatable_Body::Instance>(vm, 2)) target = vm_module::get_userdata_object<Animatable_Body::Instance>(vm, 2) -> get_node();

                vm -> push_value(target ? self -> body -> overlaps_body(target) : false);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "overlaps_area", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(area)", true)
                    .require(2, [](Machine* vm, int idx) { return vm_module::is_userdata<Area::Instance>(vm, idx); });

                auto other = vm_module::get_userdata_object<Area::Instance>(vm, 2);
                vm -> push_value(other ? self -> body -> overlaps_area(other -> get_node()) : false);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_gravity_space_override_mode", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, space_override_registry);

                auto mode = static_cast<base_class::SpaceOverride>(vm -> get_int(2));
                self -> body -> set_gravity_space_override_mode(mode);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_gravity_is_point", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_gravity_is_point(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_gravity_point_center", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(center)", true)
                    .require(2, &Machine::is_vector3);

                auto center = vm -> get_vector3(2);
                self -> body -> set_gravity_point_center(center);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_gravity_direction", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(direction)", true)
                    .require(2, &Machine::is_vector3);

                auto direction = vm -> get_vector3(2);
                self -> body -> set_gravity_direction(direction);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_gravity", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(gravity)", true)
                    .require(2, &Machine::is_number);

                auto gravity = vm -> get_float(2);
                self -> body -> set_gravity(gravity);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_priority", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(priority)", true)
                    .require(2, &Machine::is_number);

                auto priority = vm -> get_int(2);
                self -> body -> set_priority(priority);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_monitoring", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_monitoring(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_monitorable", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_monitorable(state);
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            API::Collision_Object::inject<Instance>(vm);
            vm -> scope_set_enum(base_scope, "space_override", space_override_registry);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}