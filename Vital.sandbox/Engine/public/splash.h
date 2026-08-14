/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: splash.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Splash Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/webview.h>


////////////////////////////
// Vital: Engine: Splash //
////////////////////////////

namespace Vital::Engine {
    class Splash : public godot::Control, public Tool::Base<Splash> {
        friend class Tool::Base<Splash>;
        private:
            Webview* webview = nullptr;
            godot::ColorRect* blackcover = nullptr;


            // Instantiators //
            Splash();
            ~Splash();
        public:
            static constexpr const char* Name = "Splash.engine";


            // Managers //
            bool is_visible();
            void ready();
            void show();
            void hide();


            // Events //
            void on_message(godot::String message);
    };
}
#endif