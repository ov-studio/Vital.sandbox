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
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/vehicle_body.h>
#include <Vital.sandbox/API/base/physics_body.h>


///////////////////////////////
// Vital: API: Vehicle_Body //
///////////////////////////////

namespace Vital::Sandbox::API {
    struct Vehicle_Body : vm_module {
        inline static const std::vector<std::string> base_scope = {"physics", "vehicle"};
        inline static constexpr bool has_remote = true;
        using base_class = Vital::Engine::Vehicle_Body;

        inline static const std::vector<std::pair<std::string, base_class::FreezeMode>> freeze_mode_registry = {
            { "STATIC",    base_class::FREEZE_MODE_STATIC    },
            { "KINEMATIC", base_class::FREEZE_MODE_KINEMATIC }
        };

        struct Instance : Physics_Body_Instance<Instance, base_class> {
            using Owner = Vehicle_Body;
        };
        inline static vm_registry<Instance> registry;

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
            // VehicleBody3D is a RigidBody3D specialisation — it inherits the full
            // shared rigid-body surface from Physics_Body::methods (mass, velocity,
            // damp, sleep, CCD, freeze, impulse / force helpers, etc.).
            // All the rigid-body methods that were previously re-declared here
            // verbatim are now provided by that single call — nothing to repeat.
            API::Physics_Body::methods<Instance, Physics_Body::Type::Rigid>(vm);

            // Vehicle-specific: whole-chassis engine / brake / steering.
            // Per-wheel overrides of the same properties live on Vehicle_Wheel.
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

            API::Syncable::bind_method<Instance>(vm, "set_engine_force", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(force)", true)
                    .require(2, &Machine::is_number);

                auto force = vm -> get_float(2);
                self -> body -> set_engine_force(force);
                vm -> push_value(true);
                return 1;
            });

            API::Syncable::bind_method<Instance>(vm, "set_brake", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(brake)", true)
                    .require(2, &Machine::is_number);

                auto brake = vm -> get_float(2);
                self -> body -> set_brake(brake);
                vm -> push_value(true);
                return 1;
            });

            API::Syncable::bind_method<Instance>(vm, "set_steering", [](auto vm, auto self, auto& id) -> int {
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
