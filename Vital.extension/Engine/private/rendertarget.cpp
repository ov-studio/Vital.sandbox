/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: rendertarget.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rendertarget Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.extension/Engine/public/rendertarget.h>
#include <Vital.extension/Engine/public/core.h>


//////////////////////////////////
// Vital: Engine: Rendertarget //
//////////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Rendertarget::~Rendertarget() {
        if (!viewport) return;
        if (active == this) active = nullptr;
        viewport -> queue_free();
        viewport = nullptr;
    }

    void Rendertarget::_update() {
        auto rs = Core::get_rendering_server();
        auto viewport_main = get_tree() -> get_root() -> get_viewport_rid();
        rs -> viewport_set_active(viewport_main, false);
        rs -> force_draw();
        rs -> viewport_set_active(viewport_main, true);
    }

    void Rendertarget::_draw() {
        Canvas::execute(static_cast<godot::Node2D*>(this), queue);
        instant = false;
    }


    // Managers //
    Rendertarget* Rendertarget::create(int width, int height, bool transparent) {
        auto rt = memnew(Rendertarget);
        rt -> viewport = memnew(godot::SubViewport);
        rt -> viewport -> set_size({width, height});
        rt -> viewport -> set_disable_3d(true);
        rt -> viewport -> set_transparent_background(transparent);
        rt -> viewport -> set_update_mode(godot::SubViewport::UPDATE_ALWAYS);
        rt -> viewport -> add_child(rt);
        Canvas::get_singleton() -> add_child(rt -> viewport);
        return rt;
    }

    void Rendertarget::destroy() {
        this -> queue_free();
    }

    void Rendertarget::clear(bool clear, bool instant) {
        this -> instant = instant;
        viewport -> set_clear_mode(clear ? godot::SubViewport::CLEAR_MODE_ONCE : godot::SubViewport::CLEAR_MODE_NEVER);
        if (clear) queue_redraw();
        if (instant) _update();
    }

    void Rendertarget::push(Canvas::Command command) {
        queue.push_back(command);
        queue_redraw();
        if (instant) _update();
    }


    // Checkers //
    bool Rendertarget::is_active() {
        return active && (active == this);
    }


    // Setters //
    void Rendertarget::set_active(Rendertarget* rt, bool clear, bool instant) {
        active = rt;
        if (!rt) return;
        rt -> clear(clear, instant);
    }


    // Getters //
    godot::Vector2i Rendertarget::get_size() {
        return viewport -> get_size();
    }

    godot::SubViewport* Rendertarget::get_viewport() {
        return viewport;
    }

    godot::Ref<godot::ViewportTexture> Rendertarget::get_texture() {
        return viewport -> get_texture();
    }

    Rendertarget* Rendertarget::get_active() {
        return active;
    }
}
#endif