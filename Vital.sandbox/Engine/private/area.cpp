/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: area.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Area Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/area.h>


//////////////////////////
// Vital: Engine: Area //
//////////////////////////

namespace Vital::Engine {
    // Managers //
    Area* Area::create() {
        auto body = memnew(Area);
        Engine::Core::get_singleton() -> add_child(body);
        body -> connect("body_entered",      godot::Callable(body, "_on_body_entered"));
        body -> connect("body_exited",       godot::Callable(body, "_on_body_exited"));
        body -> connect("area_entered",      godot::Callable(body, "_on_area_entered"));
        body -> connect("area_exited",       godot::Callable(body, "_on_area_exited"));
        body -> connect("body_shape_entered", godot::Callable(body, "_on_body_shape_entered"));
        body -> connect("body_shape_exited",  godot::Callable(body, "_on_body_shape_exited"));
        body -> connect("area_shape_entered", godot::Callable(body, "_on_area_shape_entered"));
        body -> connect("area_shape_exited",  godot::Callable(body, "_on_area_shape_exited"));
        return body;
    }

    void Area::destroy() {
        queue_free();
    }


    // Events //
    void Area::on_body_entered(godot::Node3D* body) {
        Tool::Event::emit("area:body_entered:" + std::to_string(reinterpret_cast<uint64_t>(this)), Tool::Stack({ body }));
    }

    void Area::on_body_exited(godot::Node3D* body) {
        Tool::Event::emit("area:body_exited:" + std::to_string(reinterpret_cast<uint64_t>(this)), Tool::Stack({ body }));
    }

    void Area::on_area_entered(godot::Area3D* area) {
        Tool::Event::emit("area:area_entered:" + std::to_string(reinterpret_cast<uint64_t>(this)), Tool::Stack({ static_cast<godot::Node3D*>(area) }));
    }

    void Area::on_area_exited(godot::Area3D* area) {
        Tool::Event::emit("area:area_exited:" + std::to_string(reinterpret_cast<uint64_t>(this)), Tool::Stack({ static_cast<godot::Node3D*>(area) }));
    }

    void Area::on_body_shape_entered(godot::RID, godot::Node3D* body, int body_shape_index, int local_shape_index) {
        Tool::Event::emit("area:body_shape_entered:" + std::to_string(reinterpret_cast<uint64_t>(this)), Tool::Stack({ body, body_shape_index, local_shape_index }));
    }

    void Area::on_body_shape_exited(godot::RID, godot::Node3D* body, int body_shape_index, int local_shape_index) {
        Tool::Event::emit("area:body_shape_exited:" + std::to_string(reinterpret_cast<uint64_t>(this)), Tool::Stack({ body, body_shape_index, local_shape_index }));
    }

    void Area::on_area_shape_entered(godot::RID, godot::Area3D* area, int area_shape_index, int local_shape_index) {
        Tool::Event::emit("area:area_shape_entered:" + std::to_string(reinterpret_cast<uint64_t>(this)), Tool::Stack({ static_cast<godot::Node3D*>(area), area_shape_index, local_shape_index }));
    }

    void Area::on_area_shape_exited(godot::RID, godot::Area3D* area, int area_shape_index, int local_shape_index) {
        Tool::Event::emit("area:area_shape_exited:" + std::to_string(reinterpret_cast<uint64_t>(this)), Tool::Stack({ static_cast<godot::Node3D*>(area), area_shape_index, local_shape_index }));
    }
}