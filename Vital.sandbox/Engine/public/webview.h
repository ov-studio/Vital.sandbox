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
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Engine/public/canvas.h>


/////////////////////////////
// Vital: Engine: Webview //
/////////////////////////////

namespace Vital::Engine {
    class Webview : public godot::Node2D {
        GDCLASS(Webview, godot::Node2D)
        public:
            struct Config {
                bool fullscreen = true;
                bool transparent = true;
                bool incognito = true;
                bool autoplay = false;
                bool zoomable = false;
            };
        protected:
            static void _bind_methods();
        private:
            godot::Control* webview = nullptr;
            std::function<void(godot::String)> message_handler;
        public:
            // Instantiators //
            Webview() : Webview(Config{}) {}
            Webview(const Config& config);
            ~Webview();


            // Managers //
            static Webview* create(const Config& config = {});
            void destroy();


            // Checkers //
            bool is_visible();
            bool is_fullscreen();
            bool is_transparent();
            bool is_autoplay();
            bool is_zoomable();
            bool is_devtools_visible();


            // Setters //
            void set_visible(bool state);
            void set_devtools_visible(bool state);
            void set_position(const godot::Vector2& position);
            void set_size(const godot::Vector2& size);
            void set_message_handler(std::function<void(godot::String)> handler);


            // Getters //
            godot::Vector2 get_position();
            godot::Vector2 get_size();


            // APIs //
            void load_url(const std::string& url);
            void load_html(const std::string& raw);
            void clear_history();
            void focus();
            void reload();
            void zoom(float value);
            void update();
            void eval(const std::string& input);
            void emit(const std::string& input);


            // Events //
            void on_message(godot::String message);
    };
}
#endif