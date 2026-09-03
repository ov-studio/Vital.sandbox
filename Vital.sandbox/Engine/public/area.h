/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: area.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Area Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>


//////////////////////////
// Vital: Engine: Area //
//////////////////////////

namespace Vital::Engine {
    class Area : public godot::Area3D {
        GDCLASS(Area, godot::Area3D)
        private:
            // Instantiators //
            Area() = default;
            ~Area() override = default;

            static void _bind_methods() {
                godot::ClassDB::bind_method(godot::D_METHOD("_on_body_entered", "body"), &Area::on_body_entered);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_body_exited",  "body"), &Area::on_body_exited);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_area_entered", "area"), &Area::on_area_entered);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_area_exited",  "area"), &Area::on_area_exited);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_body_shape_entered", "body_rid", "body", "body_shape_index", "local_shape_index"), &Area::on_body_shape_entered);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_body_shape_exited",  "body_rid", "body", "body_shape_index", "local_shape_index"), &Area::on_body_shape_exited);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_area_shape_entered", "area_rid", "area", "area_shape_index", "local_shape_index"), &Area::on_area_shape_entered);
                godot::ClassDB::bind_method(godot::D_METHOD("_on_area_shape_exited",  "area_rid", "area", "area_shape_index", "local_shape_index"), &Area::on_area_shape_exited);
            }
        public:
            // Managers //
            static Area* create();
            void destroy();


            // Events //
            void on_body_entered(godot::Node3D* body);
            void on_body_exited(godot::Node3D*  body);
            void on_area_entered(godot::Area3D* area);
            void on_area_exited(godot::Area3D*  area);
            void on_body_shape_entered(godot::RID body_rid, godot::Node3D* body, int body_shape_index, int local_shape_index);
            void on_body_shape_exited(godot::RID  body_rid, godot::Node3D* body, int body_shape_index, int local_shape_index);
            void on_area_shape_entered(godot::RID area_rid, godot::Area3D* area, int area_shape_index, int local_shape_index);
            void on_area_shape_exited(godot::RID  area_rid, godot::Area3D* area, int area_shape_index, int local_shape_index);
    };
}