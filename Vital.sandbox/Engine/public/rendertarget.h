/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: rendertarget.h
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
#include <Vital.sandbox/Engine/public/canvas.h>


//////////////////////////////////
// Vital: Engine: Rendertarget //
//////////////////////////////////

namespace Vital::Engine {
    class Rendertarget : public godot::Node2D {
        GDCLASS(Rendertarget, godot::Node2D)
        protected:
            godot::SubViewport* viewport = nullptr;
            inline static Rendertarget* active = nullptr;
        private:
            inline static uint32_t layers_used = 0;
            static constexpr int LAYER_FIRST = 20;
            static constexpr int LAYER_LAST = 31;

            Engine::Canvas::Draw_Pool pool;
            godot::Camera3D* camera = nullptr;
            bool instant = false;
            int layer_bit = -1;

            
            // Instantiators //
            Rendertarget() = default;
            ~Rendertarget() override;
            static void _bind_methods() {
                godot::ClassDB::bind_method(godot::D_METHOD("_on_frame_pre_draw"), &Rendertarget::_on_frame_pre_draw);
            }
        public:
            // Hooks //
            void _process(double delta) override;
            void _on_frame_pre_draw();


            // Managers //
            static Rendertarget* create(godot::Vector2 size, bool transparent = false, bool hdr = false);
            void enable_3d();
            void destroy();
            void clear(bool clear, bool instant);
            void update();
            void notify_drawn();


            // Checkers //
            bool is_active();
            bool is_3d() const;


            // Getters //
            static Rendertarget* get_active();
            int get_layer_bit() const;
            godot::Vector2 get_size();
            godot::SubViewport* get_viewport();
            godot::Ref<godot::ViewportTexture> get_texture();
            Engine::Canvas::Draw_Pool& get_pool();


            // Setters //
            static void set_active(Rendertarget* rendertarget = nullptr, bool clear = false, bool instant = false);
    };
}
#endif
