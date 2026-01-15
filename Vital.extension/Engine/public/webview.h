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
#include <Vital.extension/Engine/public/index.h>


////////////////////////////
// Vital: Godot: Webview //
////////////////////////////

namespace Vital::Godot {
    class Webview {
        protected:
            godot::Control* webview = nullptr;
        public:
            // Instantiators //
            Webview() = default;
            ~Webview();


            // APIs //
            void load_url(const std::string& url);
    };
}
#endif