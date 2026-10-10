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


    // Last frame //
    void Display::ensure_last_frame_texture() {
        auto rs = godot::RenderingServer::get_singleton();
        if (!last_frame_connected) {
            rs -> connect("frame_post_draw", godot::Callable(this, "_on_frame_post_draw"));
            last_frame_connected = true;
        }
        if (!last_frame_texture.is_valid()) last_frame_texture = rs -> texture_2d_placeholder_create();
        last_frame_idle = 0;
        last_frame_requested = true;
    }

    // Empty until a copy exists. Copies only happen on frames where this was requested.
    godot::RID Display::get_last_frame_texture() {
        ensure_last_frame_texture();
        return last_frame_ready ? last_frame_texture : godot::RID();
    }

    godot::RID Display::pin_last_frame_texture() {
        ensure_last_frame_texture();
        last_frame_pinned = true;
        return last_frame_texture;
    }

    void Display::_on_frame_post_draw() {
        if (last_frame_requested.exchange(false) || last_frame_pinned) {
            last_frame_idle = 0;
            viewport_rid = Core::get_scene_root() -> get_viewport_rid();
            copy_target = last_frame_texture;
            godot::RenderingServer::get_singleton() -> call_on_render_thread(godot::Callable(this, "_copy_last_frame"));
            return;
        }
        // Not requested this frame: never serve an old frame when requested again later
        last_frame_ready = false;
        if (!last_frame_texture.is_valid() || ++last_frame_idle < LAST_FRAME_IDLE_LIMIT) return;
        // Idle for a while: release the GPU memory, it is recreated on the next request
        godot::RID texture = last_frame_texture;
        last_frame_texture = godot::RID();
        last_frame_idle = 0;
        godot::RenderingServer::get_singleton() -> call_on_render_thread(godot::Callable(this, "_release_last_frame").bind(texture));
    }

    // Runs on the render thread
    void Display::_copy_last_frame() {
        auto rs = godot::RenderingServer::get_singleton();
        auto rd = rs -> get_rendering_device();
        if (!rd || !copy_target.is_valid()) return;
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
            // Swaps the content, copy_target keeps its RID (the wrapper RID passed in is consumed)
            rs -> texture_replace(copy_target, rs -> texture_rd_create(created));
            if (last_frame_rid.is_valid()) rd -> free_rid(last_frame_rid);
            last_frame_rid = created;
            last_frame_size = size;
        }
        rd -> texture_copy(source, last_frame_rid, godot::Vector3(0, 0, 0), godot::Vector3(0, 0, 0), godot::Vector3(size.x, size.y, 1), 0, 0, 0, 0);
        last_frame_ready = true;
    }

    // Runs on the render thread
    void Display::_release_last_frame(godot::RID texture) {
        auto rs = godot::RenderingServer::get_singleton();
        if (texture.is_valid()) rs -> free_rid(texture);
        auto rd = rs -> get_rendering_device();
        if (rd && last_frame_rid.is_valid()) rd -> free_rid(last_frame_rid);
        last_frame_rid = godot::RID();
        last_frame_size = godot::Vector2i();
    }

    void Display::free_last_frame() {
        last_frame_ready = false;
        auto rs = godot::RenderingServer::get_singleton();
        if (!rs) return;
        rs -> force_sync(); // let any queued copy finish before the textures go away
        if (last_frame_texture.is_valid()) rs -> free_rid(last_frame_texture);
        auto rd = rs -> get_rendering_device();
        if (rd && last_frame_rid.is_valid()) rd -> free_rid(last_frame_rid);
        last_frame_texture = godot::RID();
        last_frame_rid = godot::RID();
    }


    // Misc //
    void Display::screenshot(const std::string& base, const std::string& path) {
        auto target = Tool::to_godot_string(fmt::format("{}/{}", base, path));
        auto image = get_texture() -> get_image();
        godot::DirAccess::make_dir_recursive_absolute(target.get_base_dir());
        if (!image.is_valid()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "failed to capture screenshot");
        if (image -> save_png(target) != godot::OK) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("failed to save screenshot"));
    }
}
#endif
