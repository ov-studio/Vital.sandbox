/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: rendertarget.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Rendertarget Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/rendertarget.h>


//////////////////////////////////
// Vital: Engine: Rendertarget //
//////////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Rendertarget::~Rendertarget() {
        pool.free_all();
        if (camera) {
            auto rs = godot::RenderingServer::get_singleton();
            auto callable = godot::Callable(this, "_on_frame_pre_draw");
            if (rs && rs -> is_connected("frame_pre_draw", callable)) rs -> disconnect("frame_pre_draw", callable);
        }
        if (layer_bit >= 0) {
            layers_used &= ~(1u << layer_bit);
            layer_bit = -1;
        }
        if (!viewport) return;
        if (active == this) active = nullptr;
        viewport -> queue_free();
        viewport = nullptr;
    }


    // Hooks //
    void Rendertarget::_process(double delta) { 
        pool.end_frame(delta); 
        instant = false; 
    }

    void Rendertarget::_on_frame_pre_draw() {
        if (!camera || !camera -> is_inside_tree()) return;
        auto* source = Engine::Core::get_scene_root() -> get_camera_3d();
        if (!source) return;

        camera -> set_global_transform(source -> get_global_transform());
        switch (source -> get_projection()) {
            case godot::Camera3D::PROJECTION_ORTHOGONAL: {
                camera -> set_orthogonal(source -> get_size(), source -> get_near(), source -> get_far());
                break;
            }
            case godot::Camera3D::PROJECTION_FRUSTUM: {
                camera -> set_frustum(source -> get_size(), source -> get_frustum_offset(), source -> get_near(), source -> get_far());
                break;
            }
            default: {
                camera -> set_perspective(source -> get_fov(), source -> get_near(), source -> get_far());
                break;
            }
        }
        camera -> set_keep_aspect_mode(source -> get_keep_aspect_mode());
        camera -> set_h_offset(source -> get_h_offset());
        camera -> set_v_offset(source -> get_v_offset());
    }

    
    // Managers //
    Rendertarget* Rendertarget::create(godot::Vector2 size, bool transparent) {
        auto rendertarget = memnew(Rendertarget);
        rendertarget -> viewport = memnew(godot::SubViewport);
        rendertarget -> viewport -> set_size(size);
        rendertarget -> viewport -> set_disable_3d(true);
        rendertarget -> viewport -> set_transparent_background(transparent);
        rendertarget -> viewport -> set_update_mode(godot::SubViewport::UPDATE_ALWAYS);
        rendertarget -> viewport -> add_child(rendertarget);
        Engine::Canvas::get_singleton() -> add_child(rendertarget -> viewport);
        rendertarget -> set_process(true);
        return rendertarget;
    }

    void Rendertarget::destroy() {
        queue_free();
    }

    void Rendertarget::clear(bool clear, bool instant) {
        this -> instant = instant;
        viewport -> set_clear_mode(clear ? godot::SubViewport::CLEAR_MODE_ONCE : godot::SubViewport::CLEAR_MODE_NEVER);
        if (instant) update();
    }

    void Rendertarget::update() {
        auto rs = Engine::Core::get_rendering_server();
        auto viewport_main = Engine::Core::get_scene_root() -> get_viewport_rid();
        rs -> viewport_set_active(viewport_main, false);
        rs -> force_draw();
        rs -> viewport_set_active(viewport_main, true);
    }

    void Rendertarget::notify_drawn() {
        if (instant) update();
    }

    void Rendertarget::enable_3d() {
        if (camera) return;
        int layer = LAYER_FIRST;
        while (layer <= LAYER_LAST && (layers_used & (1u << layer))) layer++;
        if (layer > LAYER_LAST) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "no free 3D rendertarget layer left (max 12)");

        layer_bit = layer;
        layers_used |= (1u << layer);
        viewport -> set_disable_3d(false);
        viewport -> set_world_3d(Engine::Core::get_scene_root() -> get_world_3d());
        godot::Ref<godot::Environment> environment;
        environment.instantiate();
        environment -> set_background(godot::Environment::BG_CLEAR_COLOR);
        godot::Ref<godot::CameraAttributesPractical> attributes;
        attributes.instantiate();

        camera = memnew(godot::Camera3D);
        camera -> set_cull_mask(1u << layer);
        camera -> set_environment(environment);
        camera -> set_attributes(attributes);
        viewport -> add_child(camera);
        camera -> make_current();
        godot::RenderingServer::get_singleton() -> connect("frame_pre_draw", godot::Callable(this, "_on_frame_pre_draw"));
    }


    // Checkers //
    bool Rendertarget::is_active() {
        return active && (active == this);
    }

    bool Rendertarget::is_3d() const {
        return camera != nullptr;
    }


    // Getters //
    Rendertarget* Rendertarget::get_active() {
        return active;
    }

    int Rendertarget::get_layer_bit() const {
        return layer_bit;
    }

    godot::Vector2 Rendertarget::get_size() {
        return viewport -> get_size();
    }

    godot::SubViewport* Rendertarget::get_viewport() {
        return viewport;
    }

    godot::Ref<godot::ViewportTexture> Rendertarget::get_texture() {
        return viewport -> get_texture();
    }

    Engine::Canvas::Draw_Pool& Rendertarget::get_pool() {
        return pool;
    }


    // Setters //
    void Rendertarget::set_active(Rendertarget* rendertarget, bool clear, bool instant) {
        active = rendertarget;
        if (!rendertarget) return;
        rendertarget -> clear(clear, instant);
    }
}
#endif
