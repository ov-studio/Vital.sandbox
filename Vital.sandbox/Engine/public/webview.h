/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: webview.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Webview Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/canvas.h>
#include <Vital.sandbox/Engine/public/texture.h>
#include <atomic>


/////////////////////////////
// Vital: Engine: Webview //
/////////////////////////////

namespace Vital::Engine {
    class Webview : public godot::Node2D {
        GDCLASS(Webview, godot::Node2D)
        public:
            struct Options {
                int z_index = 0;
                bool fullscreen = true;
                bool transparent = true;
                bool incognito = true;
                bool autoplay = false;
                bool zoomable = false;
                bool forward_input = false;
                bool overlay = false;
                bool offscreen = false;
            };
            
            static constexpr int system_z_floor = 10000;

            using Payload = std::variant<
                std::monostate, 
                std::string, 
                bool
            >;

            inline static const std::vector<std::string> signal_registry = {
                "preload",
                "load",
                "resize",
                "message"
            };
        private:
            Options options;
            godot::Control* webview = nullptr;
            std::unordered_map<std::string, std::function<void(Payload)>> handlers;
            Vital::Engine::Texture* offscreen_texture = nullptr;
            // Instance id of the current offscreen ImageTexture, pushed here by
            // on_texture_ready() whenever the Rust side emits "texture_ready" (see
            // lib.rs: emitted from process(), the main thread, every time a new frame
            // is captured). get_texture() reads this atomic instead of doing a direct
            // `webview -> call("get_texture")` — a live cross-thread call into a
            // GDExtension object from whatever thread the sandbox/Lua VM runs on is
            // not guaranteed safe, which is why every OTHER webview method here
            // (eval, load_html, load_url, reload, zoom, post_message) already goes
            // through call_deferred() instead of a direct call. get_texture() is a
            // getter and needs a synchronous return value, so it can't use
            // call_deferred() the same way — this signal+cache is the getter-safe
            // equivalent: the write happens on the main thread (signal delivery),
            // and the read is a plain atomic load, safe from any thread.
            std::atomic<int64_t> cached_texture_id{0};
            static inline Webview* input_forwarder = nullptr;
            static inline std::vector<Webview*> buffer;


            // Instantiators //
            Webview() : Webview(Options{}) {}
            Webview(const Options& options);
            ~Webview();

            static void _bind_methods() {
                godot::ClassDB::bind_method(godot::D_METHOD("on_preload", "url"), &Webview::on_preload);
                godot::ClassDB::bind_method(godot::D_METHOD("on_load", "url"), &Webview::on_load);
                godot::ClassDB::bind_method(godot::D_METHOD("on_resize"), &Webview::on_resize);
                godot::ClassDB::bind_method(godot::D_METHOD("on_message", "message"), &Webview::on_message);
                godot::ClassDB::bind_method(godot::D_METHOD("on_texture_ready", "instance_id"), &Webview::on_texture_ready);
            }


            // Helpers //
            void pause_input_forwarder();
            static Webview* select_input_forwarder();
            static void update_input_forwarder();
        public:
            // Managers //
            static Webview* create(const Options& options = {});
            void destroy();


            // Checkers //
            bool is_visible();
            bool is_fullscreen();
            bool is_transparent();
            bool is_overlay();
            bool is_offscreen();
            bool is_incognito();
            bool is_autoplay();
            bool is_zoomable();
            bool is_forward_input();
            bool is_devtools_visible();


            // Getters //
            godot::Vector2 get_position();
            godot::Vector2 get_size();
            int get_z_index();
            Vital::Engine::Texture* get_texture();


            // Setters //
            void set_visible(bool state);
            void set_focussed(bool state);
            void set_position(const godot::Vector2& position);
            void set_size(const godot::Vector2& size);
            void set_z_index(int value);
            void set_devtools_visible(bool state);
            void set_handler(const std::string& type, std::function<void(Payload)> handler);
            void reset_handler(const std::string& type);


            // Misc //
            void load_url(const std::string& url);
            void load_html(const std::string& raw);
            void clear_history();
            void reload();
            void zoom(float value);
            void eval(const std::string& input);
            void emit(const std::string& input);
            void signal(const std::string& type, Payload payload = std::monostate{});


            // Events //
            void on_preload(godot::String url);
            void on_load(godot::String url);
            void on_resize();
            void on_message(godot::String message);
            void on_texture_ready(int64_t instance_id);
    };
}
#endif