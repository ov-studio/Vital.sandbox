/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: physics: vehicle_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vehicle Body APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/vehicle_body.h>
#include <Vital.sandbox/API/core/physics_body.h>


/////////////////////////////////////
// Vital: API: Vehicle_Body //
/////////////////////////////////////

namespace Vital::Sandbox::API {
    struct Vehicle_Body : vm_module {
        inline static const std::vector<std::string> base_scope = {"physics", "vehicle"};
        using base_class = Vital::Engine::Vehicle_Body;

        inline static const std::vector<std::pair<std::string, base_class::FreezeMode>> freeze_mode_registry = {
            { "STATIC",    base_class::FREEZE_MODE_STATIC    },
            { "KINEMATIC", base_class::FREEZE_MODE_KINEMATIC }
        };

        struct Instance : vm_instance<Instance> {
            using Owner = Vehicle_Body;
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
            vm_module::register_type<Vehicle_Body>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                auto instance = Instance::init(vm);
                instance -> body = base_class::create();
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            // VehicleBody3D is a RigidBody3D, so it carries the same shared body surface. //
            API::Physics_Body::methods<Instance, Physics_Body::Type::Rigid>(vm);

            // Rigid-body surface (mirrors Rigid_Body — VehicleBody3D inherits all of this). //
            vm_module::bind_method<Instance>(vm, "get_mass", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_mass());
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

            vm_module::bind_method<Instance>(vm, "is_using_continuous_collision_detection", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_using_continuous_collision_detection());
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

            vm_module::bind_method<Instance>(vm, "set_use_continuous_collision_detection", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_use_continuous_collision_detection(state);
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

            // Vehicle-specific: whole-chassis engine/brake/steering (per-wheel values live on Vehicle_Wheel). //
            vm_module::bind_method<Instance>(vm, "get_engine_force", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_engine_force());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_brake", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_brake());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_steering", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_steering());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_engine_force", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(force)", true)
                    .require(2, &Machine::is_number);

                auto force = vm -> get_float(2);
                self -> body -> set_engine_force(force);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_brake", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(brake)", true)
                    .require(2, &Machine::is_number);

                auto brake = vm -> get_float(2);
                self -> body -> set_brake(brake);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_steering", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(steering)", true)
                    .require(2, &Machine::is_number);

                auto steering = vm -> get_float(2);
                self -> body -> set_steering(steering);
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            API::Physics_Body::inject<Instance>(vm);
            vm -> scope_set_enum(base_scope, "freeze_mode", freeze_mode_registry);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Vehicle_Body : vm_module {};
}
#endif
