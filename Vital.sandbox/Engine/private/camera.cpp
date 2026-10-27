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
#include <Vital.sandbox/Engine/public/dimension.h>
#include <Vital.sandbox/Engine/public/mainmenu.h>


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
        Engine::Core::get_singleton() -> add_child(camera);
        camera -> attributes.instantiate();
        camera -> set_attributes(camera -> attributes);
        camera -> apply_draw_distance();
        return camera;
    }

    void Camera::destroy() {
        queue_free();
    }

    void Camera::apply_draw_distance() {
        const float mult = MainMenu::get_draw_distance();
        if (mult == applied_draw_distance) return;
        set_far(get_logical_far()*mult);
        applied_draw_distance = mult;
    }


    // Checkers //
    bool Camera::is_active() {
        return is_current();
    }


    // Getters //
    Camera* Camera::get_active() {
        auto viewport = Engine::Core::get_scene_root();
        return viewport ? godot::Object::cast_to<Camera>(viewport -> get_camera_3d()) : nullptr;
    }

    godot::Ref<godot::CameraAttributesPractical> Camera::get_attributes() const {
        return attributes;
    }

    float Camera::get_logical_far() const {
        return get_far()/(applied_draw_distance > 0.0001f ? applied_draw_distance : 1.0f);
    }


    // Setters //
    void Camera::set_active(Camera* camera) {
        if (camera) {
            camera -> apply_draw_distance();
            camera -> make_current();
        }
        else {
            auto active = get_active();
            if (active) active -> clear_current(false);
        }
        Engine::Dimension::refresh();
    }

    void Camera::set_logical_far(float z_far) {
        applied_draw_distance = MainMenu::get_draw_distance();
        set_far(z_far*applied_draw_distance);
    }
}
#endif