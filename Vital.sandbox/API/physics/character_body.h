/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: physics: character_body.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Character Body APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/character_body.h>
#include <Vital.sandbox/API/core/physics_body.h>


///////////////////////////////////
// Vital: API: Character_Body //
///////////////////////////////////

namespace Vital::Sandbox::API {
    struct Character_Body : vm_module {
        inline static const std::vector<std::string> base_scope = {"physics", "character"};
        using base_class = Vital::Engine::Character_Body;

        inline static const std::vector<std::pair<std::string, base_class::MotionMode>> motion_mode_registry = {
            { "GROUNDED", base_class::MOTION_MODE_GROUNDED },
            { "FLOATING", base_class::MOTION_MODE_FLOATING }
        };

        inline static const std::vector<std::pair<std::string, base_class::PlatformOnLeave>> platform_on_leave_registry = {
            { "ADD_VELOCITY",         base_class::PLATFORM_ON_LEAVE_ADD_VELOCITY         },
            { "ADD_UPWARD_VELOCITY",  base_class::PLATFORM_ON_LEAVE_ADD_UPWARD_VELOCITY  },
            { "DO_NOTHING",           base_class::PLATFORM_ON_LEAVE_DO_NOTHING           }
        };

        struct Instance : vm_instance<Instance> {
            using Owner = Character_Body;
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
            vm_module::register_type<Character_Body>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                auto instance = Instance::init(vm);
                instance -> body = base_class::create();
                instance -> store(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            API::Physics_Body::methods<Instance, Physics_Body::Type::Character>(vm);

            vm_module::bind_method<Instance>(vm, "get_velocity", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_velocity());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_up_direction", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_up_direction());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_motion_mode", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_motion_mode());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_floor_max_angle", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_floor_max_angle());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_on_floor", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_on_floor());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_on_ceiling", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_on_ceiling());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_on_wall", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> is_on_wall());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_floor_normal", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_floor_normal());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_wall_normal", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_wall_normal());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_real_velocity", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_real_velocity());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_platform_velocity", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_platform_velocity());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_platform_angular_velocity", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> body -> get_platform_angular_velocity());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_velocity", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(velocity)", true)
                    .require(2, &Machine::is_vector3);

                auto velocity = vm -> get_vector3(2);
                self -> body -> set_velocity(velocity);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_up_direction", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(direction)", true)
                    .require(2, &Machine::is_vector3);

                auto direction = vm -> get_vector3(2);
                self -> body -> set_up_direction(direction);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_motion_mode", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, motion_mode_registry);

                auto mode = static_cast<base_class::MotionMode>(vm -> get_int(2));
                self -> body -> set_motion_mode(mode);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_platform_on_leave", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, platform_on_leave_registry);

                auto mode = static_cast<base_class::PlatformOnLeave>(vm -> get_int(2));
                self -> body -> set_platform_on_leave(mode);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_floor_max_angle", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(radians)", true)
                    .require(2, &Machine::is_number);

                auto radians = vm -> get_float(2);
                self -> body -> set_floor_max_angle(radians);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_floor_snap_length", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(length)", true)
                    .require(2, &Machine::is_number);

                auto length = vm -> get_float(2);
                self -> body -> set_floor_snap_length(length);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_floor_stop_on_slope_enabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_floor_stop_on_slope_enabled(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_slide_on_ceiling_enabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(state)", true)
                    .require(2, &Machine::is_bool);

                auto state = vm -> get_bool(2);
                self -> body -> set_slide_on_ceiling_enabled(state);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "move_and_slide", [](auto vm, auto self, auto& id) -> int {
                auto collided = self -> body -> move_and_slide();
                vm -> push_value(collided);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "apply_floor_snap", [](auto vm, auto self, auto& id) -> int {
                self -> body -> apply_floor_snap();
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            API::Physics_Body::inject<Instance>(vm);
            vm -> scope_set_enum(base_scope, "motion_mode", motion_mode_registry);
            vm -> scope_set_enum(base_scope, "platform_on_leave", platform_on_leave_registry);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Character_Body : vm_module {};
}
#endif
