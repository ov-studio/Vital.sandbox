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
        protected:
            godot::Control* webview = nullptr;
        public:
            // Instantiators //
            Webview();
            ~Webview();


            // APIs //
            void load_from_url(const std::string& url);
            void load_from_raw(const std::string& raw);
            void clear_history();
            void eval(const std::string& input);
            void focus();
            void reload();
            void zoom(float value);
            void update();
            bool is_visible();
            void set_visible(bool state);
            bool is_fullscreen();
            void set_fullscreen(bool state);
            bool is_transparent();
            void set_transparent(bool state);
            bool is_autoplay();
            void set_autoplay(bool state);
            bool is_zoomable();
            void set_zoomable(bool state);
            bool is_devtools_visible();
            void set_devtools_visible(bool state);
            void emit(const std::string& input);
    };
}
#endif