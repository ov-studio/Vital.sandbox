/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: physics: rigid_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rigid Body APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/rigid_body.h>
#include <Vital.sandbox/API/core/physics_body.h>


/////////////////////////////
// Vital: API: Rigid_Body //
/////////////////////////////

namespace Vital::Sandbox::API {
    struct Rigid_Body : vm_module {
        inline static const std::vector<std::string> base_scope = {"physics", "rigid"};
        using base_class = Vital::Engine::Rigid_Body;

        inline static const std::vector<std::pair<std::string, base_class::FreezeMode>> freeze_mode_registry = {
            { "STATIC",    base_class::FREEZE_MODE_STATIC    },
            { "KINEMATIC", base_class::FREEZE_MODE_KINEMATIC }
        };

        inline static const std::vector<std::pair<std::string, base_class::CenterOfMassMode>> center_of_mass_mode_registry = {
            { "AUTO",   base_class::CENTER_OF_MASS_MODE_AUTO   },
            { "CUSTOM", base_class::CENTER_OF_MASS_MODE_CUSTOM }
        };

        inline static const std::vector<std::pair<std::string, base_class::DampMode>> damp_mode_registry = {
            { "COMBINE", base_class::DAMP_MODE_COMBINE },
            { "REPLACE", base_class::DAMP_MODE_REPLACE }
        };

        struct Instance : vm_instance<Instance> {
            using Owner = Rigid_Body;
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
                if (Instance::find_unlocked(instance) && (instance -> body == ptr)) return instance;
            }
            return nullptr;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Rigid_Body>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                auto instance = Instance::init(vm);
                instance -> body = base_class::create();
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Physics_Body::methods<Instance, Physics_Body::Type::Rigid>(vm);

            vm_module::bind_method<Instance>(vm, "get_mass", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_mass());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_gravity_scale", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_gravity_scale());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_linear_velocity", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_linear_velocity());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_angular_velocity", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_angular_velocity());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_linear_damp", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_linear_damp());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_angular_damp", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_angular_damp());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_sleeping", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_sleeping());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_able_to_sleep", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_able_to_sleep());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_contact_monitor_enabled", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_contact_monitor_enabled());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_max_contacts_reported", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_max_contacts_reported());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_contact_count", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_contact_count());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_using_continuous_collision_detection", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_using_continuous_collision_detection());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_lock_rotation_enabled", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_lock_rotation_enabled());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_freeze_enabled", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_freeze_enabled());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_freeze_mode", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_freeze_mode());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_colliding_bodies", [](auto vm, auto self, auto& id) -> int {
                auto bodies = self -> body -> get_colliding_bodies();
                vm -> create_table();
                for (int i = 0; i < bodies.size(); i++) {
                    auto node = godot::Object::cast_to<godot::Node3D>(bodies[i]);
                    auto other = Rigid_Body::find_by_ptr(static_cast<base_class*>(node));
                    if (other) {
                        other -> get_reference(other -> self_reference(), true, vm);
                        vm -> set_table_field(i + 1, -2);
                    }
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_mass", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mass)", true)
                    .require(2, &Machine::is_number);

                auto mass = vm -> get_float(2);
                self -> body -> set_mass(mass);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_gravity_scale", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(scale)", true)
                    .require(2, &Machine::is_number);

                auto scale = vm -> get_float(2);
                self -> body -> set_gravity_scale(scale);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_linear_velocity", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(velocity)", true)
                    .require(2, &Machine::is_vector3);

                auto velocity = vm -> get_vector3(2);
                self -> body -> set_linear_velocity(velocity);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_angular_velocity", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(velocity)", true)
                    .require(2, &Machine::is_vector3);

                auto velocity = vm -> get_vector3(2);
                self -> body -> set_angular_velocity(velocity);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_linear_damp", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(damp)", true)
                    .require(2, &Machine::is_number);

                auto damp = vm -> get_float(2);
                self -> body -> set_linear_damp(damp);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_angular_damp", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(damp)", true)
                    .require(2, &Machine::is_number);

                auto damp = vm -> get_float(2);
                self -> body -> set_angular_damp(damp);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_sleeping", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_sleeping(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_can_sleep", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_can_sleep(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_contact_monitor", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_contact_monitor(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_max_contacts_reported", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(amount)", true)
                    .require(2, &Machine::is_number);

                auto amount = vm -> get_int(2);
                self -> body -> set_max_contacts_reported(amount);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_use_continuous_collision_detection", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_use_continuous_collision_detection(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_lock_rotation_enabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_lock_rotation_enabled(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_freeze_enabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_freeze_enabled(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_freeze_mode", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, freeze_mode_registry);

                auto mode = static_cast<base_class::FreezeMode>(vm -> get_int(2));
                self -> body -> set_freeze_mode(mode);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "apply_central_impulse", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(impulse)", true)
                    .require(2, &Machine::is_vector3);

                auto impulse = vm -> get_vector3(2);
                self -> body -> apply_central_impulse(impulse);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "apply_impulse", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(impulse, position = {0, 0, 0})", true)
                    .require(2, &Machine::is_vector3)
                    .optional(3, &Machine::is_vector3);

                auto impulse = vm -> get_vector3(2);
                auto position = vm -> is_vector3(3) ? vm -> get_vector3(3) : godot::Vector3(0, 0, 0);
                self -> body -> apply_impulse(impulse, position);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "apply_torque_impulse", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(impulse)", true)
                    .require(2, &Machine::is_vector3);

                auto impulse = vm -> get_vector3(2);
                self -> body -> apply_torque_impulse(impulse);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "apply_central_force", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(force)", true)
                    .require(2, &Machine::is_vector3);

                auto force = vm -> get_vector3(2);
                self -> body -> apply_central_force(force);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "apply_force", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(force, position = {0, 0, 0})", true)
                    .require(2, &Machine::is_vector3)
                    .optional(3, &Machine::is_vector3);

                auto force = vm -> get_vector3(2);
                auto position = vm -> is_vector3(3) ? vm -> get_vector3(3) : godot::Vector3(0, 0, 0);
                self -> body -> apply_force(force, position);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "apply_torque", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(torque)", true)
                    .require(2, &Machine::is_vector3);

                auto torque = vm -> get_vector3(2);
                self -> body -> apply_torque(torque);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "add_constant_central_force", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(force)", true)
                    .require(2, &Machine::is_vector3);

                auto force = vm -> get_vector3(2);
                self -> body -> add_constant_central_force(force);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "add_constant_force", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(force, position = {0, 0, 0})", true)
                    .require(2, &Machine::is_vector3)
                    .optional(3, &Machine::is_vector3);

                auto force = vm -> get_vector3(2);
                auto position = vm -> is_vector3(3) ? vm -> get_vector3(3) : godot::Vector3(0, 0, 0);
                self -> body -> add_constant_force(force, position);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "add_constant_torque", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(torque)", true)
                    .require(2, &Machine::is_vector3);

                auto torque = vm -> get_vector3(2);
                self -> body -> add_constant_torque(torque);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_constant_force", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(force)", true)
                    .require(2, &Machine::is_vector3);

                auto force = vm -> get_vector3(2);
                self -> body -> set_constant_force(force);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_constant_torque", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(torque)", true)
                    .require(2, &Machine::is_vector3);

                auto torque = vm -> get_vector3(2);
                self -> body -> set_constant_torque(torque);
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            API::Physics_Body::inject<Instance>(vm);
            vm -> scope_set_enum(base_scope, "freeze_mode", freeze_mode_registry);
            vm -> scope_set_enum(base_scope, "center_of_mass_mode", center_of_mass_mode_registry);
            vm -> scope_set_enum(base_scope, "damp_mode", damp_mode_registry);
            vm -> scope_set_enum(base_scope, "axis", API::Physics_Body::axis_registry);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Rigid_Body : vm_module {};
}
#endif
