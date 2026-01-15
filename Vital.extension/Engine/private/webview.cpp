/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: webview.cpp
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
#include <Vital.extension/Engine/public/webview.h>


////////////////////////////
// Vital: Godot: Webview //
////////////////////////////

namespace Vital::Godot {
    // Instantiators //
    Webview::Webview() {
        godot::Object* object = godot::ClassDB::instantiate("WebView");
        if (!object) throw Vital::Error::fetch("webview-failed", "No compatible plugin found");
        else {
            webview = godot::Object::cast_to<godot::Control>(object);
            if (!webview) {
                memdelete(object);
                throw Vital::Error::fetch("webview-failed", "No compatible device found");
            }
        }
        Canvas::get_singleton() -> add_child(webview);
    }

    ~Webview::Webview() {
        if (!webview) return;
        memdelete(webview);
        webview = nullptr;
    }


    // APIs //
    void Webview::load_url(const std::string& url) {
        webview -> call("load_url", to_godot_string(url));
    }
}
#endif