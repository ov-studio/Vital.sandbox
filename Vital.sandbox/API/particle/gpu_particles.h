/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: particle: gpu_particles.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 11/10/2026
     Desc: GPU Particles APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/gpu_particles.h>
#include <Vital.sandbox/Engine/public/image.h>
#include <Vital.sandbox/API/base/node_3d.h>
#include <Vital.sandbox/API/core/image.h>
#include <Vital.sandbox/API/core/svg.h>
#include <Vital.sandbox/API/core/rendertarget.h>
#include <Vital.sandbox/API/core/model.h>
#include <Vital.sandbox/API/core/shader.h>


////////////////////////////////
// Vital: API: GPU_Particles //
////////////////////////////////

// TODO: Improve
namespace Vital::Sandbox::API {
    struct GPU_Particles : vm_module {
        inline static const std::vector<std::string> base_scope = {"particle", "gpu"};
        inline static constexpr bool has_streamed = true;
        using base_class = Vital::Engine::GPU_Particles;
        using process_material = godot::ParticleProcessMaterial;

        inline static const std::vector<std::pair<std::string, base_class::DrawOrder>> draw_order_registry = {
            { "INDEX",            base_class::DRAW_ORDER_INDEX            },
            { "LIFETIME",         base_class::DRAW_ORDER_LIFETIME         },
            { "REVERSE_LIFETIME", base_class::DRAW_ORDER_REVERSE_LIFETIME },
            { "VIEW_DEPTH",       base_class::DRAW_ORDER_VIEW_DEPTH       }
        };

        inline static const std::vector<std::pair<std::string, process_material::EmissionShape>> emission_shape_registry = {
            { "POINT",          process_material::EMISSION_SHAPE_POINT          },
            { "SPHERE",         process_material::EMISSION_SHAPE_SPHERE         },
            { "SPHERE_SURFACE", process_material::EMISSION_SHAPE_SPHERE_SURFACE },
            { "BOX",            process_material::EMISSION_SHAPE_BOX            },
            { "RING",           process_material::EMISSION_SHAPE_RING           }
        };

        inline static const std::vector<std::pair<std::string, process_material::Parameter>> param_registry = {
            { "VELOCITY",         process_material::PARAM_INITIAL_LINEAR_VELOCITY },
            { "ANGULAR_VELOCITY", process_material::PARAM_ANGULAR_VELOCITY        },
            { "ORBIT_VELOCITY",   process_material::PARAM_ORBIT_VELOCITY          },
            { "LINEAR_ACCEL",     process_material::PARAM_LINEAR_ACCEL            },
            { "RADIAL_ACCEL",     process_material::PARAM_RADIAL_ACCEL            },
            { "TANGENTIAL_ACCEL", process_material::PARAM_TANGENTIAL_ACCEL        },
            { "DAMPING",          process_material::PARAM_DAMPING                 },
            { "ANGLE",            process_material::PARAM_ANGLE                   },
            { "SCALE",            process_material::PARAM_SCALE                   },
            { "HUE_VARIATION",    process_material::PARAM_HUE_VARIATION           }
        };

        inline static const std::vector<std::pair<std::string, godot::BaseMaterial3D::BlendMode>> blend_mode_registry = {
            { "MIX", godot::BaseMaterial3D::BLEND_MODE_MIX },
            { "ADD", godot::BaseMaterial3D::BLEND_MODE_ADD },
            { "SUB", godot::BaseMaterial3D::BLEND_MODE_SUB },
            { "MUL", godot::BaseMaterial3D::BLEND_MODE_MUL }
        };

        struct Instance : vm_instance<Instance> {
            using Owner = GPU_Particles;
            base_class* particles = nullptr;

            auto get_node() {
                return particles;
            }

            bool is_streamed() const {
                return Node_3D::is_streamed(particles);
            }

            bool is_alive() const {
                return particles ? true : false;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> particles) {
                    instance -> particles -> destroy();
                    instance -> particles = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static void bind(Machine* vm) {
            vm_module::register_type<GPU_Particles>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                auto instance = Instance::init(vm);
                instance -> particles = base_class::create();

                // Signals: "particle:emit" and "particle:finished", both carry the emitter.
                // Weak reference so a pending signal never keeps a destroyed emitter alive
                std::weak_ptr<Instance> weak = instance;
                auto wire = [weak](const char* name) {
                    return [weak, name]() {
                        auto self = weak.lock();
                        if (!self || !Instance::find_unlocked(self) || !self -> is_alive()) return;
                        Manager::Sandbox::get_singleton() -> signal(name, Tool::StackValue(self));
                    };
                };
                instance -> particles -> set_emit_handler(wire("particle:emit"));
                instance -> particles -> set_finished_handler(wire("particle:finished"));

                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Node_3D::methods<Instance, Node_3D::Type::Spatial>(vm);

            // Playback //
            {
                vm_module::bind_method<Instance>(vm, "is_emitting", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> is_emitting());
                    return 1;
                });

                // Fires "particle:emit" when this starts an emission cycle
                vm_module::bind_method<Instance>(vm, "emit", [](auto vm, auto self, auto& id) -> int {
                    self -> particles -> emit();
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "stop", [](auto vm, auto self, auto& id) -> int {
                    self -> particles -> set_emitting(false);
                    vm -> push_value(true);
                    return 1;
                });

                // Fires "particle:emit", the new cycle starts straight away
                vm_module::bind_method<Instance>(vm, "restart", [](auto vm, auto self, auto& id) -> int {
                    self -> particles -> restart_emission();
                    vm -> push_value(true);
                    return 1;
                });

                // Emits a single cycle. Fires "particle:emit" now and "particle:finished" once every particle is gone,
                // destroy() the emitter from that signal for fire and forget effects
                vm_module::bind_method<Instance>(vm, "burst", [](auto vm, auto self, auto& id) -> int {
                    self -> particles -> burst();
                    vm -> push_value(true);
                    return 1;
                });
            }

            // Emitter getters //
            {
                vm_module::bind_method<Instance>(vm, "get_amount", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_amount());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_lifetime", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_lifetime());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "is_one_shot", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_one_shot());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_preprocess", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_pre_process_time());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_speed_scale", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_speed_scale());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_explosiveness", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_explosiveness_ratio());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_randomness", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_randomness_ratio());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_fixed_fps", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_fixed_fps());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "is_local_coords", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_use_local_coordinates());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_draw_order", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(static_cast<int>(self -> particles -> get_draw_order()));
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_visibility_aabb", [](auto vm, auto self, auto& id) -> int {
                    auto aabb = self -> particles -> get_visibility_aabb();
                    vm -> push_value(aabb.position);
                    vm -> push_value(aabb.size);
                    return 2;
                });
            }

            // Emitter setters //
            {
                vm_module::bind_method<Instance>(vm, "set_amount", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(amount)", true)
                        .require(2, &Machine::is_number);

                    // Changing the amount restarts the emitter
                    self -> particles -> set_amount(std::max(1, vm -> get_int(2)));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_lifetime", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(seconds)", true)
                        .require(2, &Machine::is_number);

                    self -> particles -> set_lifetime(std::max(0.01f, vm -> get_float(2)));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_one_shot", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(state)", true)
                        .require(2, &Machine::is_bool);

                    self -> particles -> set_one_shot(vm -> get_bool(2));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_preprocess", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(seconds)", true)
                        .require(2, &Machine::is_number);

                    self -> particles -> set_pre_process_time(std::max(0.0f, vm -> get_float(2)));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_speed_scale", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(scale)", true)
                        .require(2, &Machine::is_number);

                    self -> particles -> set_speed_scale(std::max(0.0f, vm -> get_float(2)));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_explosiveness", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(ratio)", true)
                        .require(2, &Machine::is_number);

                    self -> particles -> set_explosiveness_ratio(godot::Math::clamp(vm -> get_float(2), 0.0f, 1.0f));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_randomness", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(ratio)", true)
                        .require(2, &Machine::is_number);

                    self -> particles -> set_randomness_ratio(godot::Math::clamp(vm -> get_float(2), 0.0f, 1.0f));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_fixed_fps", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(fps)", true)
                        .require(2, &Machine::is_number);

                    self -> particles -> set_fixed_fps(std::max(0, vm -> get_int(2)));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_local_coords", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(state)", true)
                        .require(2, &Machine::is_bool);

                    self -> particles -> set_use_local_coordinates(vm -> get_bool(2));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_draw_order", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(order)", true)
                        .require_enum(2, draw_order_registry);

                    self -> particles -> set_draw_order(static_cast<base_class::DrawOrder>(vm -> get_int(2)));
                    vm -> push_value(true);
                    return 1;
                });

                // Particles outside of this box are culled, raise it when particles travel far from the emitter
                vm_module::bind_method<Instance>(vm, "set_visibility_aabb", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(position, size)", true)
                        .require(2, &Machine::is_vector3)
                        .require(3, &Machine::is_vector3);

                    self -> particles -> set_visibility_aabb(godot::AABB(vm -> get_vector3(2), vm -> get_vector3(3)));
                    vm -> push_value(true);
                    return 1;
                });
            }

            // Process getters //
            {
                vm_module::bind_method<Instance>(vm, "get_direction", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_process_params() -> get_direction());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_spread", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_process_params() -> get_spread());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_flatness", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_process_params() -> get_flatness());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_gravity", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_process_params() -> get_gravity());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_color", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_process_params() -> get_color());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_param", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(param)", true)
                        .require_enum(2, param_registry);

                    auto param = static_cast<process_material::Parameter>(vm -> get_int(2));
                    auto params = self -> particles -> get_process_params();
                    vm -> push_value(params -> get_param_min(param));
                    vm -> push_value(params -> get_param_max(param));
                    return 2;
                });

                vm_module::bind_method<Instance>(vm, "get_emission_shape", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(static_cast<int>(self -> particles -> get_process_params() -> get_emission_shape()));
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_lifetime_randomness", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_process_params() -> get_lifetime_randomness());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "is_turbulence_enabled", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_process_params() -> get_turbulence_enabled());
                    return 1;
                });
            }

            // Process setters //
            {
                vm_module::bind_method<Instance>(vm, "set_direction", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(direction)", true)
                        .require(2, &Machine::is_vector3);

                    self -> particles -> get_process_params() -> set_direction(vm -> get_vector3(2));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_spread", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(degrees)", true)
                        .require(2, &Machine::is_number);

                    self -> particles -> get_process_params() -> set_spread(godot::Math::clamp(vm -> get_float(2), 0.0f, 180.0f));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_flatness", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(amount)", true)
                        .require(2, &Machine::is_number);

                    self -> particles -> get_process_params() -> set_flatness(godot::Math::clamp(vm -> get_float(2), 0.0f, 1.0f));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_gravity", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(gravity)", true)
                        .require(2, &Machine::is_vector3);

                    self -> particles -> get_process_params() -> set_gravity(vm -> get_vector3(2));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_color", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(color)", true)
                        .require(2, &Machine::is_color);

                    self -> particles -> get_process_params() -> set_color(vm -> get_color(2));
                    vm -> push_value(true);
                    return 1;
                });

                // set_param(param, min, max = min)
                vm_module::bind_method<Instance>(vm, "set_param", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(param, min, max = min)", true)
                        .require_enum(2, param_registry)
                        .require(3, &Machine::is_number)
                        .optional(4, &Machine::is_number);

                    auto param = static_cast<process_material::Parameter>(vm -> get_int(2));
                    auto min = vm -> get_float(3);
                    auto max = vm -> is_number(4) ? vm -> get_float(4) : min;
                    auto params = self -> particles -> get_process_params();
                    params -> set_param_min(param, std::min(min, max));
                    params -> set_param_max(param, std::max(min, max));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_lifetime_randomness", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(ratio)", true)
                        .require(2, &Machine::is_number);

                    self -> particles -> get_process_params() -> set_lifetime_randomness(godot::Math::clamp(vm -> get_float(2), 0.0f, 1.0f));
                    vm -> push_value(true);
                    return 1;
                });

                // set_emission_shape(shape, a, b, c)
                // SPHERE / SPHERE_SURFACE: radius | BOX: extents | RING: radius, inner_radius, height
                vm_module::bind_method<Instance>(vm, "set_emission_shape", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(shape, radius = 1, inner_radius = 0, height = 1)", true)
                        .require_enum(2, emission_shape_registry)
                        .optional(3, [](Machine* vm, int idx) { return vm -> is_number(idx) || vm -> is_vector3(idx); })
                        .optional(4, &Machine::is_number)
                        .optional(5, &Machine::is_number);

                    auto shape = static_cast<process_material::EmissionShape>(vm -> get_int(2));
                    auto params = self -> particles -> get_process_params();
                    params -> set_emission_shape(shape);
                    if (shape == process_material::EMISSION_SHAPE_BOX) {
                        params -> set_emission_box_extents(vm -> is_vector3(3) ? vm -> get_vector3(3) : godot::Vector3(1, 1, 1));
                    }
                    else if (shape == process_material::EMISSION_SHAPE_SPHERE || shape == process_material::EMISSION_SHAPE_SPHERE_SURFACE) {
                        params -> set_emission_sphere_radius(vm -> is_number(3) ? vm -> get_float(3) : 1.0f);
                    }
                    else if (shape == process_material::EMISSION_SHAPE_RING) {
                        params -> set_emission_ring_radius(vm -> is_number(3) ? vm -> get_float(3) : 1.0f);
                        params -> set_emission_ring_inner_radius(vm -> is_number(4) ? vm -> get_float(4) : 0.0f);
                        params -> set_emission_ring_height(vm -> is_number(5) ? vm -> get_float(5) : 1.0f);
                    }
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_turbulence", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(state, strength = 1, scale = 1.5)", true)
                        .require(2, &Machine::is_bool)
                        .optional(3, &Machine::is_number)
                        .optional(4, &Machine::is_number);

                    auto params = self -> particles -> get_process_params();
                    params -> set_turbulence_enabled(vm -> get_bool(2));
                    if (vm -> is_number(3)) params -> set_turbulence_noise_strength(vm -> get_float(3));
                    if (vm -> is_number(4)) params -> set_turbulence_noise_scale(vm -> get_float(4));
                    vm -> push_value(true);
                    return 1;
                });

                // Fades the particle color over its lifetime, e.g. set_color_ramp({1, 0.6, 0.1, 1}, {1, 0.1, 0, 0})
                vm_module::bind_method<Instance>(vm, "set_color_ramp", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(from, to)", true)
                        .require(2, &Machine::is_color)
                        .require(3, &Machine::is_color);

                    self -> particles -> set_color_ramp(vm -> get_color(2), vm -> get_color(3));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "reset_color_ramp", [](auto vm, auto self, auto& id) -> int {
                    self -> particles -> reset_color_ramp();
                    vm -> push_value(true);
                    return 1;
                });

                // Scales the particle over its lifetime, multiplies with the SCALE param
                vm_module::bind_method<Instance>(vm, "set_scale_curve", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(from, to)", true)
                        .require(2, &Machine::is_number)
                        .require(3, &Machine::is_number);

                    self -> particles -> set_scale_curve(std::max(0.0f, vm -> get_float(2)), std::max(0.0f, vm -> get_float(3)));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "reset_scale_curve", [](auto vm, auto self, auto& id) -> int {
                    self -> particles -> reset_scale_curve();
                    vm -> push_value(true);
                    return 1;
                });
            }

            // Draw //
            {
                vm_module::bind_method<Instance>(vm, "get_size", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_draw_size());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "is_billboard", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> is_billboard());
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "get_blend_mode", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(static_cast<int>(self -> particles -> get_draw_material() -> get_blend_mode()));
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "is_unshaded", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> get_draw_material() -> get_shading_mode() == godot::BaseMaterial3D::SHADING_MODE_UNSHADED);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_size", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(size)", true)
                        .require(2, &Machine::is_vector2);

                    self -> particles -> set_draw_size(vm -> get_vector2(2));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_billboard", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(state)", true)
                        .require(2, &Machine::is_bool);

                    self -> particles -> set_billboard(vm -> get_bool(2));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_blend_mode", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(mode)", true)
                        .require_enum(2, blend_mode_registry);

                    self -> particles -> get_draw_material() -> set_blend_mode(static_cast<godot::BaseMaterial3D::BlendMode>(vm -> get_int(2)));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "set_unshaded", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(state)", true)
                        .require(2, &Machine::is_bool);

                    self -> particles -> get_draw_material() -> set_shading_mode(vm -> get_bool(2) ? godot::BaseMaterial3D::SHADING_MODE_UNSHADED : godot::BaseMaterial3D::SHADING_MODE_PER_PIXEL);
                    vm -> push_value(true);
                    return 1;
                });

                // Ribbon trail instead of a quad, lifetime is how long the trail stays behind each particle
                vm_module::bind_method<Instance>(vm, "set_trail", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(state, lifetime = 0.3)", true)
                        .require(2, &Machine::is_bool)
                        .optional(3, &Machine::is_number);

                    auto lifetime = vm -> is_number(3) ? vm -> get_float(3) : 0.3f;
                    self -> particles -> set_trail(vm -> get_bool(2), std::max(0.01f, lifetime));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "is_trail_enabled", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> is_trail_mode());
                    return 1;
                });

                // Ribbon behind the emitter's own movement, independent of the particles (stop() them if only the ribbon is wanted)
                vm_module::bind_method<Instance>(vm, "set_motion_trail", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(state, lifetime = 0.5)", true)
                        .require(2, &Machine::is_bool)
                        .optional(3, &Machine::is_number);

                    auto lifetime = vm -> is_number(3) ? vm -> get_float(3) : 0.5f;
                    self -> particles -> set_motion_trail(vm -> get_bool(2), std::max(0.01f, lifetime));
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "is_motion_trail_enabled", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> is_motion_trail());
                    return 1;
                });
            }

            // Material //
            {
                // set_material(material)
                // material — image, svg, rendertarget or a file path: shown through the built-in material
                //            (billboard, blend mode, unshaded and the particle color keep working)
                //            SPATIAL shader: replaces the built-in material, draw it however you like
                vm_module::bind_method<Instance>(vm, "set_material", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(material)", true)
                        .require(2, [](Machine* vm, int idx) {
                            return vm -> is_string(idx)
                                || vm_module::is_userdata<API::Image::Instance>(vm, idx)
                                || vm_module::is_userdata<API::SVG::Instance>(vm, idx)
                                || vm_module::is_userdata<API::Rendertarget::Instance>(vm, idx)
                                || vm_module::is_userdata<API::Shader::Instance>(vm, idx);
                        });

                    if (vm -> is_string(2)) {
                        self -> particles -> set_material(vm -> get_string(2));
                    }
                    else if (vm_module::is_userdata<API::Image::Instance>(vm, 2)) {
                        auto image = vm_module::get_userdata_object<API::Image::Instance>(vm, 2);
                        self -> particles -> set_material(image -> get_node());
                    }
                    else if (vm_module::is_userdata<API::SVG::Instance>(vm, 2)) {
                        auto svg = vm_module::get_userdata_object<API::SVG::Instance>(vm, 2);
                        self -> particles -> set_material(svg -> get_node());
                    }
                    else if (vm_module::is_userdata<API::Rendertarget::Instance>(vm, 2)) {
                        auto rendertarget = vm_module::get_userdata_object<API::Rendertarget::Instance>(vm, 2);
                        self -> particles -> set_material(rendertarget -> get_node());
                    }
                    else {
                        auto shader = vm_module::get_userdata_object<API::Shader::Instance>(vm, 2);
                        self -> particles -> set_material(shader -> get_node());
                    }
                    vm -> push_value(true);
                    return 1;
                });

                // Back to the plain built-in material
                vm_module::bind_method<Instance>(vm, "reset_material", [](auto vm, auto self, auto& id) -> int {
                    self -> particles -> reset_material();
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "has_shader", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> has_draw_shader());
                    return 1;
                });

                // set_mesh(model, component = all)
                // Draws every particle as the model's mesh instead of a quad. Without a component all of the model's
                // components are used (max 4, one draw pass each). The model keeps existing, destroy or hide it if
                // it is only a mesh source. Billboarding turns off, size comes from the SCALE param.
                // The mesh keeps its own materials unless set_material is used
                vm_module::bind_method<Instance>(vm, "set_mesh", [](auto vm, auto self, auto& id) -> int {
                    vm_args(vm, id, "(model, component = all)", true)
                        .require(2, [](Machine* vm, int idx) { return vm_module::is_userdata<API::Model::Instance>(vm, idx); })
                        .optional(3, &Machine::is_string);

                    auto model = vm_module::get_userdata_object<API::Model::Instance>(vm, 2);
                    self -> particles -> set_mesh(model -> get_node(), vm -> is_string(3) ? vm -> get_string(3) : "");
                    vm -> push_value(true);
                    return 1;
                });

                // Back to the quad
                vm_module::bind_method<Instance>(vm, "reset_mesh", [](auto vm, auto self, auto& id) -> int {
                    self -> particles -> reset_mesh();
                    vm -> push_value(true);
                    return 1;
                });

                vm_module::bind_method<Instance>(vm, "has_mesh", [](auto vm, auto self, auto& id) -> int {
                    vm -> push_value(self -> particles -> has_mesh());
                    return 1;
                });
            }
        }

        static void inject(Machine* vm) {
            API::Node_3D::inject<Instance>(vm);
            vm -> scope_set_enum(base_scope, "draw_order", draw_order_registry);
            vm -> scope_set_enum(base_scope, "emission_shape", emission_shape_registry);
            vm -> scope_set_enum(base_scope, "param", param_registry);
            vm -> scope_set_enum(base_scope, "blend_mode", blend_mode_registry);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct GPU_Particles : vm_module {};
}
#endif
