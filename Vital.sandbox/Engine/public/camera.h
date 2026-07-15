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


            // Getters //
            static Camera* get_active();
            godot::Ref<godot::CameraAttributesPractical> get_attributes() const;


            // Setters //
            static void set_active(Camera* camera = nullptr);
    };
}
#endif
