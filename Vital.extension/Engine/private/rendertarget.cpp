/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: rendertarget.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rendertarget Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#if defined(Vital_SDK_Client)
#pragma once
#include <Vital.extension/Engine/public/rendertarget.h>
#include <Vital.extension/Engine/public/core.h>


/////////////////////////////////
// Vital: Godot: RenderTarget //
/////////////////////////////////

namespace Vital::Godot {
    // Instantiators //
    RenderTarget::~RenderTarget() {
        if (!viewport) return;
        viewport -> queue_free();
        viewport = nullptr;
    }

    void RenderTarget::_update() {
        if (!is_inside_tree()) return;
        auto* rs = Core::get_rendering_server();
        auto viewport_main = get_tree() -> get_root() -> get_viewport_rid();
        rs -> viewport_set_active(viewport_main, false);
        rs -> force_draw();
        rs -> viewport_set_active(viewport_main, true);
    }

    void RenderTarget::_draw() {
        if (!viewport -> has_transparent_background()) draw_rect(godot::Rect2({0, 0}, get_size()), godot::Color(0, 0, 0, 1), true, -1, false);
        Canvas::execute(static_cast<godot::Node2D*>(this), queue);
        instant = false;
    }


    // Getters //
    godot::Vector2i RenderTarget::get_size() {
        return viewport -> get_size();
    }

    godot::SubViewport* RenderTarget::get_viewport() {
        return viewport;
    }

    godot::Ref<godot::ViewportTexture> RenderTarget::get_texture() {
        return viewport -> get_texture();
    }

    RenderTarget* RenderTarget::get_rendertarget() {
        return rendertarget;
    }


    // APIs //
    RenderTarget* RenderTarget::create(int width, int height, bool transparent) {
        auto* rt = memnew(RenderTarget);
        rt -> viewport = memnew(godot::SubViewport);
        rt -> viewport -> set_size({width, height});
        rt -> viewport -> set_disable_3d(true);
        rt -> viewport -> set_transparent_background(transparent);
        rt -> viewport -> set_update_mode(godot::SubViewport::UPDATE_ALWAYS);
        rt -> viewport -> call_deferred("add_child", rt);
        Canvas::get_singleton() -> call_deferred("add_child", rt -> viewport);
        return rt;
    }

    void RenderTarget::set_rendertarget(RenderTarget* rt, bool clear, bool instant) {
        rendertarget = rt;
        if (!rt) return;
        rt -> clear(clear, instant);
    }

    void RenderTarget::clear(bool clear, bool instant) {
        get_viewport() -> set_clear_mode(clear ? godot::SubViewport::CLEAR_MODE_ONCE : godot::SubViewport::CLEAR_MODE_NEVER);
        this -> instant = instant;
        if (clear) {
            queue.clear();
            queue_redraw();
        }
        if (instant) _update();
    }

    void RenderTarget::push(Canvas::Command command) {
        queue.push_back(command);
        queue_redraw();
        if (instant) _update();
    }
}
#endif