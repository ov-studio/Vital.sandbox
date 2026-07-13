/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: camera.cpp
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
#include <Vital.sandbox/Engine/public/camera.h>


////////////////////////////
// Vital: Engine: Camera //
////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Camera::~Camera() {
        if (is_current()) clear_current();
    }


    // Managers //
    Camera* Camera::create() {
        auto camera = memnew(Camera);
        Engine::Core::get_scene_root() -> add_child(camera);
        camera -> attributes.instantiate();
        camera -> set_attributes(camera -> attributes);
        return camera;
    }

    void Camera::destroy() {
        queue_free();
    }


    // Checkers //
    bool Camera::is_active() {
        return is_current();
    }

    bool Camera::is_auto_exposure_enabled() {
        return attributes -> is_auto_exposure_enabled();
    }

    bool Camera::is_dof_blur_far_enabled() {
        return attributes -> is_dof_blur_far_enabled();
    }

    bool Camera::is_dof_blur_near_enabled() {
        return attributes -> is_dof_blur_near_enabled();
    }


    // Getters //
    Camera* Camera::get_active() {
        auto viewport = Engine::Core::get_scene_root() -> get_viewport();
        return viewport ? godot::Object::cast_to<Camera>(viewport -> get_camera_3d()) : nullptr;
    }

    float Camera::get_auto_exposure_scale() {
        return attributes -> get_auto_exposure_scale();
    }

    float Camera::get_auto_exposure_speed() {
        return attributes -> get_auto_exposure_speed();
    }

    float Camera::get_auto_exposure_min_sensitivity() {
        return attributes -> get_auto_exposure_min_sensitivity();
    }

    float Camera::get_auto_exposure_max_sensitivity() {
        return attributes -> get_auto_exposure_max_sensitivity();
    }

    float Camera::get_dof_blur_amount() {
        return attributes -> get_dof_blur_amount();
    }

    float Camera::get_dof_blur_far_distance() {
        return attributes -> get_dof_blur_far_distance();
    }

    float Camera::get_dof_blur_far_transition() {
        return attributes -> get_dof_blur_far_transition();
    }

    float Camera::get_dof_blur_near_distance() {
        return attributes -> get_dof_blur_near_distance();
    }

    float Camera::get_dof_blur_near_transition() {
        return attributes -> get_dof_blur_near_transition();
    }


    // Setters //
    void Camera::set_active(Camera* camera) {
        if (camera) {
            camera -> make_current();
            return;
        }
        auto active = get_active();
        if (active) active -> clear_current(false);
    }

    void Camera::set_auto_exposure_enabled(bool state) {
        attributes -> set_auto_exposure_enabled(state);
    }

    void Camera::set_auto_exposure_scale(float value) {
        attributes -> set_auto_exposure_scale(value);
    }

    void Camera::set_auto_exposure_speed(float value) {
        attributes -> set_auto_exposure_speed(value);
    }

    void Camera::set_auto_exposure_min_sensitivity(float value) {
        attributes -> set_auto_exposure_min_sensitivity(value);
    }

    void Camera::set_auto_exposure_max_sensitivity(float value) {
        attributes -> set_auto_exposure_max_sensitivity(value);
    }

    void Camera::set_dof_blur_amount(float value) {
        attributes -> set_dof_blur_amount(value);
    }

    void Camera::set_dof_blur_far_enabled(bool state) {
        attributes -> set_dof_blur_far_enabled(state);
    }

    void Camera::set_dof_blur_far_distance(float value) {
        attributes -> set_dof_blur_far_distance(value);
    }

    void Camera::set_dof_blur_far_transition(float value) {
        attributes -> set_dof_blur_far_transition(value);
    }

    void Camera::set_dof_blur_near_enabled(bool state) {
        attributes -> set_dof_blur_near_enabled(state);
    }

    void Camera::set_dof_blur_near_distance(float value) {
        attributes -> set_dof_blur_near_distance(value);
    }

    void Camera::set_dof_blur_near_transition(float value) {
        attributes -> set_dof_blur_near_transition(value);
    }
}
#endif
