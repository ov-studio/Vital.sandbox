/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: camera.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Camera Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>
#include <godot_cpp/classes/camera3d.hpp>


////////////////////////////
// Vital: Engine: Camera //
////////////////////////////

namespace Vital::Engine {
    class Camera : public godot::Camera3D {
        GDCLASS(Camera, godot::Camera3D)
        private:
            godot::Ref<godot::CameraAttributesPractical> attributes;
            
            // Instantiators //
            Camera() = default;
            ~Camera() override;
            static void _bind_methods() {}
        public:
            // Managers //
            static Camera* create();
            void destroy();


            // Checkers //
            bool is_active();
            bool is_auto_exposure_enabled();
            bool is_dof_blur_far_enabled();
            bool is_dof_blur_near_enabled();


            // Getters //
            static Camera* get_active();
            float get_auto_exposure_scale();
            float get_auto_exposure_speed();
            float get_auto_exposure_min_sensitivity();
            float get_auto_exposure_max_sensitivity();
            float get_dof_blur_amount();
            float get_dof_blur_far_distance();
            float get_dof_blur_far_transition();
            float get_dof_blur_near_distance();
            float get_dof_blur_near_transition();


            // Setters //
            static void set_active(Camera* camera = nullptr);
            void set_auto_exposure_enabled(bool state);
            void set_auto_exposure_scale(float value);
            void set_auto_exposure_speed(float value);
            void set_auto_exposure_min_sensitivity(float value);
            void set_auto_exposure_max_sensitivity(float value);
            void set_dof_blur_amount(float value);
            void set_dof_blur_far_enabled(bool state);
            void set_dof_blur_far_distance(float value);
            void set_dof_blur_far_transition(float value);
            void set_dof_blur_near_enabled(bool state);
            void set_dof_blur_near_distance(float value);
            void set_dof_blur_near_transition(float value);
    };
}
#endif
