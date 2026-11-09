/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: vehicle_wheel.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vehicle Wheel Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/syncable.h>


///////////////////////////////////
// Vital: Engine: Vehicle_Wheel //
///////////////////////////////////

namespace Vital::Engine {
    class Vehicle_Wheel : public godot::VehicleWheel3D, public Engine::ISyncChild {
        GDCLASS(Vehicle_Wheel, godot::VehicleWheel3D)
        friend class Network;
        private:
            #if defined(VSDK_Client)
            struct DebugLayer {
                godot::MeshInstance3D* node = nullptr;
                godot::Ref<godot::ImmediateMesh> mesh;
            };
            #endif

            #if defined(VSDK_Client)
            inline static bool debug_all = false;
            inline static std::mutex mutex;
            inline static std::unordered_set<Vehicle_Wheel*> buffer;
            #endif

            int wheel_id = -1;
            #if defined(VSDK_Client)
            bool debug_visible = false;
            std::optional<godot::Transform3D> rest_transform;
            DebugLayer debug_wheel;
            DebugLayer debug_suspension;
            godot::Ref<godot::StandardMaterial3D> debug_material;
            #endif


            // Instantiators //
            Vehicle_Wheel();
            ~Vehicle_Wheel() override;
            static void _bind_methods() {}


            // Helpers //
            #if defined(VSDK_Client)
            void update_debug();
            #endif
        public:
            // Hooks //
            void _notification(int what);
            #if defined(VSDK_Client)
            void _process(double delta) override;
            #endif


            // Managers //
            static Vehicle_Wheel* create(godot::Node3D* body);
            void destroy();
            void refresh();
            void sync();


            // Checkers //
            #if defined(VSDK_Client)
            bool is_debug_visible() const;
            static bool is_debug_all();
            #endif


            // Getters //
            int get_wheel_id() const;
            uint32_t get_parent_net_id() const;


            // Setters //
            void set_wheel_id(int index);
            #if defined(VSDK_Client)
            void set_debug_visible(bool state);
            static void set_debug_all(bool state);
            #endif
    };
}
