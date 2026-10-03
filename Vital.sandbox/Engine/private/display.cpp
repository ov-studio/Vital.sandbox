/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: display.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Display Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/display.h>


/////////////////////////////
// Vital: Engine: Display //
/////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    // Getters //
    godot::Vector2 Display::get_resolution() {
        return Core::get_display_server() -> window_get_size();
    }

    godot::Ref<godot::ViewportTexture> Display::get_texture() {
        return Core::get_scene_root() -> get_texture();
    }


    // Drawing the root viewport's own texture inside that viewport is a read/write
    // feedback loop (speckles). hint_screen_texture reads the back buffer copy instead.
    godot::Ref<godot::ShaderMaterial> Display::get_screen_material() {
        if (!screen_material.is_valid()) {
            godot::Ref<godot::Shader> shader;
            shader.instantiate();
            shader -> set_code(
                "shader_type canvas_item;\n"
                "uniform sampler2D screen_tex : hint_screen_texture, filter_linear, repeat_disable;\n"
                "void fragment() { COLOR = vec4(texture(screen_tex, UV).rgb, 1.0)*COLOR; }\n"
            );
            screen_material.instantiate();
            screen_material -> set_shader(shader);
        }
        return screen_material;
    }


    // Returns an empty ref until the first copy landed (nothing is drawn meanwhile).
    // Copies only happen on frames where this was requested.
    godot::RID Display::get_last_frame_texture() {
        if (!last_frame_connected) {
            godot::RenderingServer::get_singleton() -> connect("frame_post_draw", godot::Callable(this, "_on_frame_post_draw"));
            last_frame_connected = true;
        }
        last_frame_requested = true;
        return last_frame_ready ? last_frame_texture : godot::RID();
    }

    void Display::_on_frame_post_draw() {
        if (!last_frame_requested.exchange(false)) return;
        viewport_rid = Core::get_scene_root() -> get_viewport_rid();
        godot::RenderingServer::get_singleton() -> call_on_render_thread(godot::Callable(this, "_copy_last_frame"));
    }

    // Runs on the render thread
    void Display::_copy_last_frame() {
        auto rs = godot::RenderingServer::get_singleton();
        auto rd = rs -> get_rendering_device();
        if (!rd) return;
        godot::RID source = rs -> texture_get_rd_texture(rs -> viewport_get_texture(viewport_rid));
        if (!source.is_valid()) return;
        auto source_format = rd -> texture_get_format(source);
        if (!source_format.is_valid()) return;
        godot::Vector2i size = {(int)source_format -> get_width(), (int)source_format -> get_height()};
        if (!last_frame_rid.is_valid() || size != last_frame_size) {
            godot::Ref<godot::RDTextureFormat> format;
            format.instantiate();
            format -> set_format(source_format -> get_format());
            format -> set_width(size.x);
            format -> set_height(size.y);
            format -> set_usage_bits(static_cast<int64_t>(
                godot::RenderingDevice::TEXTURE_USAGE_SAMPLING_BIT
                | godot::RenderingDevice::TEXTURE_USAGE_CAN_COPY_TO_BIT
                | godot::RenderingDevice::TEXTURE_USAGE_CAN_COPY_FROM_BIT
            ));
            godot::Ref<godot::RDTextureView> view;
            view.instantiate();
            godot::RID created = rd -> texture_create(format, view);
            if (!created.is_valid()) return;
            godot::RID wrapped = rs -> texture_rd_create(created);
            last_frame_ready = false;
            if (last_frame_texture.is_valid()) rs -> free_rid(last_frame_texture);
            if (last_frame_rid.is_valid()) rd -> free_rid(last_frame_rid);
            last_frame_texture = wrapped;
            last_frame_rid = created;
            last_frame_size = size;
        }
        rd -> texture_copy(source, last_frame_rid, godot::Vector3(0, 0, 0), godot::Vector3(0, 0, 0), godot::Vector3(size.x, size.y, 1), 0, 0, 0, 0);
        last_frame_ready = true;
    }

    void Display::free_last_frame() {
        last_frame_ready = false;
        auto rs = godot::RenderingServer::get_singleton();
        if (!rs) return;
        if (last_frame_texture.is_valid()) rs -> free_rid(last_frame_texture);
        auto rd = rs -> get_rendering_device();
        if (rd && last_frame_rid.is_valid()) rd -> free_rid(last_frame_rid);
        last_frame_texture = godot::RID();
        last_frame_rid = godot::RID();
    }


    // Misc //
    void Display::screenshot(const std::string& base, const std::string& path) {
        auto target = Tool::to_godot_string(base + "/" + path);
        auto image = get_texture() -> get_image();
        godot::DirAccess::make_dir_recursive_absolute(target.get_base_dir());
        if (!image.is_valid()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "failed to capture screenshot");
        if (image -> save_png(target) != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("failed to save screenshot"));
    }
}
#endif
