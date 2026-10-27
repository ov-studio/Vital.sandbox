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


////////////////////////////
// Vital: Engine: Camera //
////////////////////////////

namespace Vital::Engine {
    class Camera : public godot::Camera3D {
        GDCLASS(Camera, godot::Camera3D)
        private:
            godot::Ref<godot::CameraAttributesPractical> attributes;
            float applied_draw_distance = 1.0f;

            
            // Instantiators //
            Camera() = default;
            ~Camera() override;
            static void _bind_methods() {}
        public:
            // Managers //
            static Camera* create();
            void destroy();
            void apply_draw_distance();


            // Checkers //
            bool is_active();


            // Getters //
            static Camera* get_active();
            godot::Ref<godot::CameraAttributesPractical> get_attributes() const;
            float get_logical_far() const;


            // Setters //
            static void set_active(Camera* camera = nullptr);
            void set_logical_far(float z_far);
    };
}
#endif
