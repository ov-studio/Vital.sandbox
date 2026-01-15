/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: webview.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Webview Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#if defined(Vital_SDK_Client)
#pragma once
#include <Vital.extension/Engine/public/canvas.h>


////////////////////////////
// Vital: Godot: Webview //
////////////////////////////

namespace Vital::Godot {
    class Webview {
        private:
            godot::Control* webview = nullptr;
        public:
            // Instantiators //
            Webview();
            ~Webview();


            // Getters //
            bool is_visible();
            bool is_fullscreen();
            bool is_transparent();
            bool is_autoplay();
            bool is_zoomable();
            bool is_devtools_visible();
            godot::Vector2 get_position();
            godot::Vector2 get_size();


            // Setters //
            void set_visible(bool state);
            void set_fullscreen(bool state);
            void set_transparent(bool state);
            void set_autoplay(bool state);
            void set_zoomable(bool state);
            void set_devtools_visible(bool state);
            void set_position(const godot::Vector2& position);
            void set_size(const godot::Vector2& size);


            // APIs //
            void load_from_url(const std::string& url);
            void load_from_raw(const std::string& raw);
            void clear_history();
            void focus();
            void reload();
            void zoom(float value);
            void update();
            void eval(const std::string& input);
            void emit(const std::string& input);
    };
}
#endif