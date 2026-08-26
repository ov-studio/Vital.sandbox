/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: physics: vehicle_wheel.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vehicle Wheel APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/vehicle_wheel.h>
#include <Vital.sandbox/API/core/node_3d.h>
#include <Vital.sandbox/API/physics/vehicle_body.h>
#include <Vital.sandbox/API/physics/area.h>


////////////////////////////////
// Vital: API: Vehicle_Wheel //
////////////////////////////////

namespace Vital::Sandbox::API {
    struct Vehicle_Wheel : vm_module {
        inline static const std::vector<std::string> base_scope = {"physics", "vehicle_wheel"};
        using base_class = Vital::Engine::Vehicle_Wheel;

        struct Instance : vm_instance<Instance> {
            using Owner = Vehicle_Wheel;
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
            vm_module::register_type<Vehicle_Wheel>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(owner)", true)
                    .require(1, [](Machine* vm, int idx) { return vm_module::is_userdata<Vehicle_Body::Instance>(vm, idx); });

                auto owner = vm_module::get_userdata_object<Vehicle_Body::Instance>(vm, 1);
                auto instance = Instance::init(vm);
                instance -> body = base_class::create(owner -> get_node());
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Node_3D::methods<Instance, Node_3D::Type::Spatial>(vm);

            vm_module::bind_method<Instance>(vm, "get_radius", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_radius());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_suspension_rest_length", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_suspension_rest_length());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_suspension_travel", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_suspension_travel());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_suspension_stiffness", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_suspension_stiffness());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_suspension_max_force", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_suspension_max_force());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_damping_compression", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_damping_compression());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_damping_relaxation", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_damping_relaxation());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_used_as_traction", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_used_as_traction());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_used_as_steering", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_used_as_steering());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_friction_slip", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_friction_slip());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_in_contact", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_in_contact());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_contact_body", [](auto vm, auto self, auto& id) -> int {
                auto node = self -> body -> get_contact_body();
                if (!node || !Area::push_node_instance(vm, node)) {
                    vm -> push_value(false);
                    return 1;
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_contact_point", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_contact_point());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_contact_normal", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_contact_normal());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_roll_influence", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_roll_influence());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_skidinfo", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_skidinfo());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_rpm", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_rpm());
                return 1;
            });

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

            vm_module::bind_method<Instance>(vm, "set_radius", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(radius)", true)
                    .require(2, &Machine::is_number);

                auto radius = vm -> get_float(2);
                self -> body -> set_radius(radius);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_suspension_rest_length", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(length)", true)
                    .require(2, &Machine::is_number);

                auto length = vm -> get_float(2);
                self -> body -> set_suspension_rest_length(length);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_suspension_travel", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(length)", true)
                    .require(2, &Machine::is_number);

                auto length = vm -> get_float(2);
                self -> body -> set_suspension_travel(length);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_suspension_stiffness", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(stiffness)", true)
                    .require(2, &Machine::is_number);

                auto stiffness = vm -> get_float(2);
                self -> body -> set_suspension_stiffness(stiffness);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_suspension_max_force", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(force)", true)
                    .require(2, &Machine::is_number);

                auto force = vm -> get_float(2);
                self -> body -> set_suspension_max_force(force);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_damping_compression", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(damping)", true)
                    .require(2, &Machine::is_number);

                auto damping = vm -> get_float(2);
                self -> body -> set_damping_compression(damping);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_damping_relaxation", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(damping)", true)
                    .require(2, &Machine::is_number);

                auto damping = vm -> get_float(2);
                self -> body -> set_damping_relaxation(damping);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_use_as_traction", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_use_as_traction(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_use_as_steering", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_use_as_steering(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_friction_slip", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(slip)", true)
                    .require(2, &Machine::is_number);

                auto slip = vm -> get_float(2);
                self -> body -> set_friction_slip(slip);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_roll_influence", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(influence)", true)
                    .require(2, &Machine::is_number);

                auto influence = vm -> get_float(2);
                self -> body -> set_roll_influence(influence);
                vm -> push_value(true);
                return 1;
            });

            // Per-wheel overrides — VehicleWheel3D exposes its own engine/brake/steering too,
            // distinct from the whole-chassis values on Vehicle_Body. //
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
            API::Node_3D::inject<Instance>(vm);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Vehicle_Wheel : vm_module {};
}
#endif
