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
    class Camera : public godot::Camera3D { // TODO: Use camera 3d? or node3d and wrap camera within it?
        GDCLASS(Camera, godot::Camera3D)
        private:
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


            // Getters //
            static Camera* get_active();


            // Setters //
            static void set_active(Camera* camera = nullptr);
    };
}
#endif
