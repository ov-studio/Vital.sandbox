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
            // Checkers //
            vm_module::bind_method<Instance>(vm, "is_active", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> is_active());
                return 1;
            });

            // Projection //
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

            vm_module::bind_method<Instance>(vm, "unproject_position", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(world_point)", true)
                    .require(2, &Machine::is_vector3);

                vm -> push_value(self -> camera -> unproject_position(vm -> get_vector3(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "project_position", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(screen_point, z_depth)", true)
                    .require(2, &Machine::is_vector2)
                    .require(3, &Machine::is_number);

                vm -> push_value(self -> camera -> project_position(vm -> get_vector2(2), vm -> get_float(3)));
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

            // Modes //
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

            vm_module::bind_method<Instance>(vm, "set_frustum", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(size, offset, z_near, z_far)", true)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_vector2)
                    .require(4, &Machine::is_number)
                    .require(5, &Machine::is_number);

                self -> camera -> set_frustum(vm -> get_float(2), vm -> get_vector2(3), vm -> get_float(4), vm -> get_float(5));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_projection", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_projection());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_projection", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, base_class::PROJECTION_PERSPECTIVE, base_class::PROJECTION_FRUSTUM);

                self -> camera -> set_projection(static_cast<base_class::ProjectionType>(vm -> get_int(2)));
                vm -> push_value(true);
                return 1;
            });

            // Current //
            vm_module::bind_method<Instance>(vm, "make_current", [](auto vm, auto self, auto& id) -> int {
                self -> camera -> make_current();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "clear_current", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(enable_next = true)", true)
                    .optional(2, &Machine::is_bool);

                self -> camera -> clear_current(vm -> is_bool(2) ? vm -> get_bool(2) : true);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_current", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(enabled)", true)
                    .require(2, &Machine::is_bool);

                self -> camera -> set_current(vm -> get_bool(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "is_current", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> is_current());
                return 1;
            });

            // Lens //
            vm_module::bind_method<Instance>(vm, "get_fov", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_fov());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_fov", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(fov)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> set_fov(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_size", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_size());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_size", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(size)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> set_size(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_near", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_near());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_near", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(z_near)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> set_near(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_far", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_far());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_far", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(z_far)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> set_far(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_frustum_offset", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_frustum_offset());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_frustum_offset", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(offset)", true)
                    .require(2, &Machine::is_vector2);

                self -> camera -> set_frustum_offset(vm -> get_vector2(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_h_offset", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_h_offset());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_h_offset", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(offset)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> set_h_offset(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_v_offset", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_v_offset());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_v_offset", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(offset)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> set_v_offset(vm -> get_float(2));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_keep_aspect_mode", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_keep_aspect_mode());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_keep_aspect_mode", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, base_class::KEEP_WIDTH, base_class::KEEP_HEIGHT);

                self -> camera -> set_keep_aspect_mode(static_cast<base_class::KeepAspect>(vm -> get_int(2)));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_doppler_tracking", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> camera -> get_doppler_tracking());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_doppler_tracking", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mode)", true)
                    .require_enum(2, base_class::DOPPLER_TRACKING_DISABLED, base_class::DOPPLER_TRACKING_PHYSICS_STEP);

                self -> camera -> set_doppler_tracking(static_cast<base_class::DopplerTracking>(vm -> get_int(2)));
                vm -> push_value(true);
                return 1;
            });

            // Culling //
            vm_module::bind_method<Instance>(vm, "get_cull_mask", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(static_cast<int64_t>(self -> camera -> get_cull_mask()));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_cull_mask", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(mask)", true)
                    .require(2, &Machine::is_number);

                self -> camera -> set_cull_mask(static_cast<uint32_t>(vm -> get_int(2)));
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_cull_mask_value", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(layer_number)", true)
                    .require(2, &Machine::is_number);

                vm -> push_value(self -> camera -> get_cull_mask_value(vm -> get_int(2)));
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_cull_mask_value", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(layer_number, value)", true)
                    .require(2, &Machine::is_number)
                    .require(3, &Machine::is_bool);

                self -> camera -> set_cull_mask_value(vm -> get_int(2), vm -> get_bool(3));
                vm -> push_value(true);
                return 1;
            });

            // Environment / Attributes / Compositor //
            // NOTE: these three wire the camera to other engine resources that need to already
            // be exposed as their own sandbox instance modules (Environment, CameraAttributes,
            // Compositor) before these can bind correctly. `resource` below is a placeholder
            // member name on each Instance -- swap in whatever the real field is called once
            // those modules exist / are confirmed.
            vm_module::bind_method<Instance>(vm, "set_environment", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(environment)", true)
                    .optional(2, [](Machine* vm, int idx) { return vm_module::is_userdata<API::Environment::Instance>(vm, idx); });

                std::shared_ptr<API::Environment::Instance> env = vm -> is_nil(2) ? nullptr : vm_module::get_userdata_object<API::Environment::Instance>(vm, 2);
                self -> camera -> set_environment(env ? env -> resource : nullptr);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_environment", [](auto vm, auto self, auto& id) -> int {
                auto env = self -> camera -> get_environment();
                auto instance = API::Environment::find_by_ptr(env.ptr());
                if (!instance) vm -> push_value(false);
                else instance -> push_self(vm);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_attributes", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(attributes)", true)
                    .optional(2, [](Machine* vm, int idx) { return vm_module::is_userdata<API::CameraAttributes::Instance>(vm, idx); });

                std::shared_ptr<API::CameraAttributes::Instance> attributes = vm -> is_nil(2) ? nullptr : vm_module::get_userdata_object<API::CameraAttributes::Instance>(vm, 2);
                self -> camera -> set_attributes(attributes ? attributes -> resource : nullptr);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_attributes", [](auto vm, auto self, auto& id) -> int {
                auto attributes = self -> camera -> get_attributes();
                auto instance = API::CameraAttributes::find_by_ptr(attributes.ptr());
                if (!instance) vm -> push_value(false);
                else instance -> push_self(vm);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "set_compositor", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(compositor)", true)
                    .optional(2, [](Machine* vm, int idx) { return vm_module::is_userdata<API::Compositor::Instance>(vm, idx); });

                std::shared_ptr<API::Compositor::Instance> compositor = vm -> is_nil(2) ? nullptr : vm_module::get_userdata_object<API::Compositor::Instance>(vm, 2);
                self -> camera -> set_compositor(compositor ? compositor -> resource : nullptr);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "get_compositor", [](auto vm, auto self, auto& id) -> int {
                auto compositor = self -> camera -> get_compositor();
                auto instance = API::Compositor::find_by_ptr(compositor.ptr());
                if (!instance) vm -> push_value(false);
                else instance -> push_self(vm);
                return 1;
            });

            // NOTE: get_camera_transform, get_camera_projection, get_frustum, get_camera_rid,
            // and get_pyramid_shape_rid are intentionally omitted -- Mixin::push_value has no
            // overload yet for Transform3D, Projection, RID, or TypedArray<Plane>. Add those
            // push_value overloads first, then these can be bound the same way as the rest.
        }

        static void inject(Machine* vm) {
            // TODO: SEPARATE REGISTRY AT THE TOP
            vm -> scope_set_enum(base_scope, "projection", std::vector<std::pair<std::string, base_class::ProjectionType>>{
                {"PERSPECTIVE", base_class::PROJECTION_PERSPECTIVE},
                {"ORTHOGONAL",  base_class::PROJECTION_ORTHOGONAL},
                {"FRUSTUM",     base_class::PROJECTION_FRUSTUM},
            });

            vm -> scope_set_enum(base_scope, "keep_aspect", std::vector<std::pair<std::string, base_class::KeepAspect>>{
                {"WIDTH",  base_class::KEEP_WIDTH},
                {"HEIGHT", base_class::KEEP_HEIGHT},
            });

            vm -> scope_set_enum(base_scope, "doppler_tracking", std::vector<std::pair<std::string, base_class::DopplerTracking>>{
                {"DISABLED",     base_class::DOPPLER_TRACKING_DISABLED},
                {"IDLE_STEP",    base_class::DOPPLER_TRACKING_IDLE_STEP},
                {"PHYSICS_STEP", base_class::DOPPLER_TRACKING_PHYSICS_STEP},
            });
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