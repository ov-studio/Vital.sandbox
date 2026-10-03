/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: display.h
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
#include <Vital.sandbox/Engine/public/core.h>


/////////////////////////////
// Vital: Engine: Display //
/////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    class Display : public godot::Object, public Tool::Base<Display> {
        GDCLASS(Display, godot::Object)
        friend class Tool::Base<Display>;
        public:
            static constexpr const char* Name = "Display.engine";
        private:
            godot::Ref<godot::ShaderMaterial> screen_material;

            // Last frame: GPU-only copy of the root viewport taken in frame_post_draw.
            // last_frame_texture is a stable RenderingServer texture RID (main thread owned),
            // its content is swapped with texture_replace so shader bindings never go stale.
            // last_frame_rid/last_frame_size are render thread only.
            static constexpr int LAST_FRAME_IDLE_LIMIT = 120;
            godot::RID viewport_rid;
            godot::RID copy_target;
            godot::RID last_frame_rid;
            godot::RID last_frame_texture;
            godot::Vector2i last_frame_size;
            std::atomic<bool> last_frame_requested { false };
            std::atomic<bool> last_frame_ready { false };
            std::atomic<bool> last_frame_pinned { false };
            bool last_frame_connected = false;
            int last_frame_idle = 0;

            void ensure_last_frame_texture();
            void free_last_frame();
            void _on_frame_post_draw();
            void _copy_last_frame();
            void _release_last_frame(godot::RID texture);


            // Instantiators //
            Display() = default;
            ~Display() override { free_last_frame(); }

            static void _bind_methods() {
                godot::ClassDB::bind_method(godot::D_METHOD("_on_frame_post_draw"), &Display::_on_frame_post_draw);
                godot::ClassDB::bind_method(godot::D_METHOD("_copy_last_frame"), &Display::_copy_last_frame);
                godot::ClassDB::bind_method(godot::D_METHOD("_release_last_frame", "texture"), &Display::_release_last_frame);
            }
        public:
            // Getters //
            godot::Vector2 get_resolution();
            godot::Ref<godot::ViewportTexture> get_texture();
            godot::Ref<godot::ShaderMaterial> get_screen_material();
            godot::RID get_last_frame_texture();   // per-frame draw use, empty until a copy exists
            godot::RID pin_last_frame_texture();   // persistent binding (shader param), copied every frame from now on


            // Misc //
            void screenshot(const std::string& base, const std::string& path);
    };
}
#endif
