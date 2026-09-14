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
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Manager/public/network.h>
#include <Vital.sandbox/Engine/public/syncable.h>
#include <Vital.sandbox/Engine/public/vehicle_wheel.h>
#include <Vital.sandbox/API/base/node_3d.h>
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
            #if !defined(VSDK_Client)
            bool pending_spawn = false;
            bool pending_transform = false;
            std::vector<std::pair<std::string, godot::Variant>> pending_configs;
            #endif

            auto get_node() {
                return body;
            }

            bool is_alive() const {
                return body ? true : false;
            }

            uint32_t get_parent_net_id() const {
                if (!body) return 0;
                auto* parent = body -> get_parent();
                if (!parent) return 0;
                auto* syncable = dynamic_cast<Vital::Engine::ISyncable*>(godot::Object::cast_to<godot::Object>(parent));
                return syncable ? syncable -> get_net_id() : 0;
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

            #if !defined(VSDK_Client)
            void flush_pending_broadcasts() {
                uint32_t nid = get_parent_net_id();
                if (nid == 0 || !body) return;
                auto* net = Manager::Network::get_singleton() -> get_node();
                if (!net) return;

                if (pending_spawn) {
                    net -> rpc("_spawn_wheel", (int)nid, body -> get_wheel_id(), body -> get_position(), body -> get_rotation());
                    pending_spawn = false;
                }
                for (auto& [key, value] : pending_configs) {
                    net -> rpc("_sync_wheel_config", (int)nid, body -> get_wheel_id(), godot::String(key.c_str()), value);
                }
                pending_configs.clear();
                if (pending_transform) {
                    net -> rpc("_sync_wheel_transform", (int)nid, body -> get_wheel_id(), body -> get_position(), body -> get_rotation());
                    pending_transform = false;
                }
            }
            #endif

            void broadcast_config(const char* key, godot::Variant value) {
                #if !defined(VSDK_Client)
                uint32_t nid = get_parent_net_id();
                if (nid == 0) {
                    pending_configs.emplace_back(key, value);
                    return;
                }
                auto* net = Manager::Network::get_singleton() -> get_node();
                if (net) net -> rpc("_sync_wheel_config", (int)nid, body -> get_wheel_id(), godot::String(key), value);
                #endif
            }

            void broadcast_transform() {
                #if !defined(VSDK_Client)
                uint32_t nid = get_parent_net_id();
                if (nid == 0) {
                    pending_transform = true;
                    return;
                }
                auto* net = Manager::Network::get_singleton() -> get_node();
                if (net) net -> rpc("_sync_wheel_transform", (int)nid, body -> get_wheel_id(), body -> get_position(), body -> get_rotation());
                #endif
            }
        };
        inline static vm_registry<Instance> registry;

        static void init(Machine* vm) {
            static Tool::Event::event_id destroyed_binding = 0;
            static Tool::Event::event_id ready_binding = 0;
            if (destroyed_binding) Tool::Event::unbind("entity:unspawned", destroyed_binding);
            if (ready_binding) Tool::Event::unbind("entity:ready", ready_binding);

            destroyed_binding = Tool::Event::bind("entity:unspawned", [](Tool::Stack args) {
                if (args.array.size() < 1) return;
                if (!args.array[0].is_raw_ptr<Vital::Engine::Vehicle_Wheel>()) return;

                auto* entity = args.array[0].as_raw_ptr<Vital::Engine::Vehicle_Wheel>();
                Vehicle_Wheel::Instance::destroy_by_ptr(entity, [](std::shared_ptr<Vehicle_Wheel::Instance> inst) {
                    inst -> body = nullptr;
                });
            });

            #if !defined(VSDK_Client)
            ready_binding = Tool::Event::bind("entity:ready", [](Tool::Stack args) {
                if (args.array.size() < 1) return;
                auto* entity = args.array[0].as<godot::Node3D*>();
                if (!entity) return;

                std::lock_guard<std::mutex> lock(Vehicle_Wheel::registry.mutex);
                for (auto& [uid, inst] : Vehicle_Wheel::registry.buffer) {
                    if (!inst || !inst -> body) continue;
                    if (inst -> body -> get_parent() != entity) continue;
                    inst -> flush_pending_broadcasts();
                }
            });
            #endif
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Vehicle_Wheel>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(owner)", true)
                    .require(1, [](Machine* vm, int idx) { return vm_module::is_userdata<Vehicle_Body::Instance>(vm, idx); });

                auto owner = vm_module::get_userdata_object<Vehicle_Body::Instance>(vm, 1);
                auto instance = Instance::init(vm);
                instance -> body = base_class::create(owner -> get_node());
                auto* entity = owner -> get_node();
                int idx_count = 0;
                for (int i = 0; i < entity -> get_child_count(); i++) {
                    if (godot::Object::cast_to<Vital::Engine::Vehicle_Wheel>(entity -> get_child(i))) idx_count++;
                }
                instance -> body -> set_wheel_id(idx_count - 1);
                #if !defined(VSDK_Client)
                uint32_t nid = instance -> get_parent_net_id();
                if (nid != 0) {
                    auto* net = Manager::Network::get_singleton() -> get_node();
                    if (net) net -> rpc("_spawn_wheel", (int)nid, instance -> body -> get_wheel_id(), instance -> body -> get_position(), instance -> body -> get_rotation());
                }
                else instance -> pending_spawn = true;
                #endif
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Node_3D::methods<Instance, Node_3D::Type::Spatial>(vm);

            vm_module::bind_method<Instance>(vm, "set_position", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(position)", true)
                    .require(2, &Machine::is_vector3);

                auto position = vm -> get_vector3(2);
                self -> body -> set_position(position);
                self -> broadcast_transform();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "translate", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(offset)", true)
                    .require(2, &Machine::is_vector3);

                auto offset = vm -> get_vector3(2);
                self -> body -> translate(offset);
                self -> broadcast_transform();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "translate_local", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(offset)", true)
                    .require(2, &Machine::is_vector3);

                auto offset = vm -> get_vector3(2);
                self -> body -> translate_object_local(offset);
                self -> broadcast_transform();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_rotation", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(rotation)", true)
                    .require(2, &Machine::is_vector3);

                auto rotation = vm -> get_vector3(2);
                self -> body -> set_rotation(rotation);
                self -> broadcast_transform();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_rotation_degrees", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(rotation)", true)
                    .require(2, &Machine::is_vector3);

                auto rotation = vm -> get_vector3(2);
                self -> body -> set_rotation_degrees(rotation);
                self -> broadcast_transform();
                vm -> push_value(true);
                return 1;
            });

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
                if (!node || !Area::push_entity(vm, node)) vm -> push_value(false);
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
                self -> broadcast_config("radius", radius);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_suspension_rest_length", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(length)", true)
                    .require(2, &Machine::is_number);

                auto length = vm -> get_float(2);
                self -> body -> set_suspension_rest_length(length);
                self -> broadcast_config("suspension_rest_length", length);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_suspension_travel", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(length)", true)
                    .require(2, &Machine::is_number);

                auto length = vm -> get_float(2);
                self -> body -> set_suspension_travel(length);
                self -> broadcast_config("suspension_travel", length);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_suspension_stiffness", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(stiffness)", true)
                    .require(2, &Machine::is_number);

                auto stiffness = vm -> get_float(2);
                self -> body -> set_suspension_stiffness(stiffness);
                self -> broadcast_config("suspension_stiffness", stiffness);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_suspension_max_force", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(force)", true)
                    .require(2, &Machine::is_number);

                auto force = vm -> get_float(2);
                self -> body -> set_suspension_max_force(force);
                self -> broadcast_config("suspension_max_force", force);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_damping_compression", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(damping)", true)
                    .require(2, &Machine::is_number);

                auto damping = vm -> get_float(2);
                self -> body -> set_damping_compression(damping);
                self -> broadcast_config("damping_compression", damping);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_damping_relaxation", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(damping)", true)
                    .require(2, &Machine::is_number);

                auto damping = vm -> get_float(2);
                self -> body -> set_damping_relaxation(damping);
                self -> broadcast_config("damping_relaxation", damping);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_use_as_traction", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_use_as_traction(state);
                self -> broadcast_config("use_as_traction", state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_use_as_steering", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_use_as_steering(state);
                self -> broadcast_config("use_as_steering", state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_friction_slip", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(slip)", true)
                    .require(2, &Machine::is_number);

                auto slip = vm -> get_float(2);
                self -> body -> set_friction_slip(slip);
                self -> broadcast_config("friction_slip", slip);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_roll_influence", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(influence)", true)
                    .require(2, &Machine::is_number);

                auto influence = vm -> get_float(2);
                self -> body -> set_roll_influence(influence);
                self -> broadcast_config("roll_influence", influence);
                vm -> push_value(true);
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
            API::Node_3D::inject<Instance>(vm);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
