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
        return camera;
    }

    void Camera::destroy() {
        queue_free();
    }


    // Checkers //
    bool Camera::is_active() {
        return is_current();
    }


    // Getters //
    Camera* Camera::get_active() {
        auto viewport = Engine::Core::get_scene_root() -> get_viewport();
        return viewport ? godot::Object::cast_to<Camera>(viewport -> get_camera_3d()) : nullptr;
    }


    // Setters //
    void Camera::set_active(Camera* camera) {
        if (camera) {
            camera -> make_current();
            return;
        }
        auto active = get_active();
        if (active) active -> clear_current();
    }
}
#endif
