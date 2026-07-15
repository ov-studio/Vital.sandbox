/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: camera.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Camera APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/camera.h>


/////////////////////////
// Vital: API: Camera //
/////////////////////////

namespace Vital::Sandbox::API {
    struct Camera : vm_module {
        inline static const std::vector<std::string> base_scope = {"core", "camera"};
        using base_class = Vital::Engine::Camera;

        inline static const std::vector<std::pair<std::string, base_class::ProjectionType>> projection_registry = {
            { "PERSPECTIVE", base_class::PROJECTION_PERSPECTIVE },
            { "ORTHOGONAL",  base_class::PROJECTION_ORTHOGONAL }
        };

        inline static const std::vector<std::pair<std::string, base_class::KeepAspect>> keep_aspect_registry = {
            { "WIDTH",  base_class::KEEP_WIDTH },
            { "HEIGHT", base_class::KEEP_HEIGHT }
        };

        inline static const std::vector<std::pair<std::string, base_class::DopplerTracking>> doppler_tracking_registry = {
            { "DISABLED",     base_class::DOPPLER_TRACKING_DISABLED },
            { "IDLE_STEP",    base_class::DOPPLER_TRACKING_IDLE_STEP },
            { "PHYSICS_STEP", base_class::DOPPLER_TRACKING_PHYSICS_STEP }
        };

        struct Instance : vm_instance<Instance> {
            using Owner = Camera;
            base_class* camera = nullptr;

            bool is_alive() const {
                return camera ? true : false;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> camera) {
                    instance -> camera -> destroy();
                    instance -> camera = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static std::shared_ptr<Instance> find_by_ptr(base_class* ptr) {
            if (!ptr) return nullptr;
            std::lock_guard<std::mutex> lock(registry.mutex);
            for (auto& [id, instance] : registry.buffer) {
                if (Instance::find_unlocked(instance) && (instance -> camera == ptr)) return instance;
            }
            return nullptr;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Camera>(vm);

            API::bind(vm, base_scope, "create", [](auto vm, auto& id) -> int {
                auto instance = Instance::init(vm);
                instance -> camera = base_class::create();
                instance -> store(true);
                return 1;
            });

            API::bind(vm, base_scope, "get_active", [](auto vm, auto& id) -> int {
                auto ptr = base_class::get_active();
                auto instance = find_by_ptr(ptr);
                if (!instance) vm -> push_value(false);
                else instance -> push_self(vm);
                return 1;
            });

            API::bind(vm, base_scope, "set_active", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(camera = nil)")
                    .optional(1, [](Machine* vm, int idx) { return vm_module::is_userdata<Instance>(vm, idx); });

                std::shared_ptr<Instance> instance = vm -> is_nil(1) ? nullptr : vm_module::get_userdata_object<Instance>(vm, 1);
                base_class::set_active(instance ? instance -> camera : nullptr);
                vm -> push_value(true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "is_active", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> is_active());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_position_behind", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(world_point)", true)
                    .require(2, &Machine::is_vector3);

                vm -> push_value(self -> camera -> is_position_behind(vm -> get_vector3(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_position_in_frustum", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(world_point)", true)
                    .require(2, &Machine::is_vector3);

                vm -> push_value(self -> camera -> is_position_in_frustum(vm -> get_vector3(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_auto_exposure_enabled", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_attributes() -> is_auto_exposure_enabled());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_dof_blur_far_enabled", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_attributes() -> is_dof_blur_far_enabled());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_dof_blur_near_enabled", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_attributes() -> is_dof_blur_near_enabled());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_camera_transform", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_camera_transform());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_camera_projection", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_camera_projection());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_projection", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_projection());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_fov", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_fov());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_size", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_size());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_near", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_near());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_far", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_far());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_h_offset", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_h_offset());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_v_offset", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_v_offset());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_keep_aspect_mode", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_keep_aspect_mode());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_doppler_tracking", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_doppler_tracking());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_auto_exposure_scale", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_attributes() -> get_auto_exposure_scale());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_auto_exposure_speed", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_attributes() -> get_auto_exposure_speed());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_auto_exposure_min_sensitivity", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_attributes() -> get_auto_exposure_min_sensitivity());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_auto_exposure_max_sensitivity", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_attributes() -> get_auto_exposure_max_sensitivity());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_dof_blur_amount", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_attributes() -> get_dof_blur_amount());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_dof_blur_far_distance", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_attributes() -> get_dof_blur_far_distance());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_dof_blur_far_transition", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_attributes() -> get_dof_blur_far_transition());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_dof_blur_near_distance", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_attributes() -> get_dof_blur_near_distance());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_dof_blur_near_transition", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_attributes() -> get_dof_blur_near_transition());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_active", [](auto vm, auto self, auto& id) -> int {
                base_class::set_active(self -> camera);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_projection", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, projection_registry);

                self -> camera -> set_projection(static_cast<base_class::ProjectionType>(vm -> get_int(2)));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_perspective", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(fov, z_near, z_far)", true)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_number)
                    .require(4, &Machine::is_number);

                self -> camera -> set_perspective(vm -> get_float(2), vm -> get_float(3), vm -> get_float(4));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_orthogonal", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(size, z_near, z_far)", true)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_number)
                    .require(4, &Machine::is_number);

                self -> camera -> set_orthogonal(vm -> get_float(2), vm -> get_float(3), vm -> get_float(4));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_fov", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(fov)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> set_fov(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_size", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(size)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> set_size(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_near", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(z_near)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> set_near(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_far", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(z_far)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> set_far(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_h_offset", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(offset)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> set_h_offset(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_v_offset", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(offset)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> set_v_offset(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_keep_aspect_mode", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, keep_aspect_registry);

                self -> camera -> set_keep_aspect_mode(static_cast<base_class::KeepAspect>(vm -> get_int(2)));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_doppler_tracking", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, doppler_tracking_registry);

                self -> camera -> set_doppler_tracking(static_cast<base_class::DopplerTracking>(vm -> get_int(2)));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_auto_exposure_enabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(enabled)", true)
                    .require(2, &Machine::is_bool);

                self -> camera -> get_attributes() -> set_auto_exposure_enabled(vm -> get_bool(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_auto_exposure_scale", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(scale)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> get_attributes() -> set_auto_exposure_scale(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_auto_exposure_speed", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(speed)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> get_attributes() -> set_auto_exposure_speed(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_auto_exposure_min_sensitivity", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(sensitivity)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> get_attributes() -> set_auto_exposure_min_sensitivity(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_auto_exposure_max_sensitivity", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(sensitivity)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> get_attributes() -> set_auto_exposure_max_sensitivity(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_dof_blur_amount", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(amount)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> get_attributes() -> set_dof_blur_amount(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_dof_blur_far_enabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(enabled)", true)
                    .require(2, &Machine::is_bool);

                self -> camera -> get_attributes() -> set_dof_blur_far_enabled(vm -> get_bool(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_dof_blur_far_distance", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(distance)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> get_attributes() -> set_dof_blur_far_distance(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_dof_blur_far_transition", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(distance)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> get_attributes() -> set_dof_blur_far_transition(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_dof_blur_near_enabled", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(enabled)", true)
                    .require(2, &Machine::is_bool);

                self -> camera -> get_attributes() -> set_dof_blur_near_enabled(vm -> get_bool(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_dof_blur_near_distance", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(distance)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> get_attributes() -> set_dof_blur_near_distance(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_dof_blur_near_transition", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(distance)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> get_attributes() -> set_dof_blur_near_transition(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "project_ray_normal", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(screen_point)", true)
                    .require(2, &Machine::is_vector2);

                vm -> push_value(self -> camera -> project_ray_normal(vm -> get_vector2(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "project_local_ray_normal", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(screen_point)", true)
                    .require(2, &Machine::is_vector2);

                vm -> push_value(self -> camera -> project_local_ray_normal(vm -> get_vector2(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "project_ray_origin", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(screen_point)", true)
                    .require(2, &Machine::is_vector2);

                vm -> push_value(self -> camera -> project_ray_origin(vm -> get_vector2(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "project_position", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(screen_point, z_depth)", true)
                    .require(2, &Machine::is_vector2)
                    .require(3, &Machine::is_number);

                vm -> push_value(self -> camera -> project_position(vm -> get_vector2(2), vm -> get_float(3)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "unproject_position", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(world_point)", true)
                    .require(2, &Machine::is_vector3);

                vm -> push_value(self -> camera -> unproject_position(vm -> get_vector3(2)));
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "projection", projection_registry);
            vm -> scope_set_enum(base_scope, "keep_aspect", keep_aspect_registry);
            vm -> scope_set_enum(base_scope, "doppler_tracking", doppler_tracking_registry);
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Camera : vm_module {};
}
#endif