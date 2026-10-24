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

// TODO: Improve
namespace Vital::Engine {
    class Vehicle_Wheel : public godot::VehicleWheel3D, public ISyncChild {
        GDCLASS(Vehicle_Wheel, godot::VehicleWheel3D)
        friend class Network;
        private:
            int wheel_id = -1;

            #if defined(VSDK_Client)
            // Debug visuals: per-wheel ring/spokes (wheel space) + suspension/ray/contact (body space)
            inline static bool debug_all = false;
            inline static std::mutex mutex;
            inline static std::unordered_set<Vehicle_Wheel*> buffer;
            bool debug_visible = false;
            bool has_rest_transform = false;
            godot::Transform3D rest_transform;
            godot::MeshInstance3D* debug_wheel_node = nullptr;
            godot::MeshInstance3D* debug_suspension_node = nullptr;
            godot::Ref<godot::ImmediateMesh> debug_wheel_mesh;
            godot::Ref<godot::ImmediateMesh> debug_suspension_mesh;
            godot::Ref<godot::StandardMaterial3D> debug_material;
            void update_debug();
            #endif


            // Instantiators //
            Vehicle_Wheel();
            ~Vehicle_Wheel() override;
            static void _bind_methods() {}
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
            void sync_inputs();


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
