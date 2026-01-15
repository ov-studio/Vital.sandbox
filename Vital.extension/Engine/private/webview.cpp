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
        Canvas::get_singleton() -> call_deferred("add_child", webview);
        webview -> call_deferred("load_url", "https://github.com/ov-studio/Vital.sandbox");
    }

    Webview::~Webview() {
        if (!webview) return;
        memdelete(webview);
        webview = nullptr;
    }


    // APIs //
    void Webview::load_from_url(const std::string& url) {
        webview -> call_deferred("load_url", to_godot_string(url));
    }

    void Webview::load_from_raw(const std::string& raw) {
        webview -> call_deferred("load_html", to_godot_string(raw));
    }

    void Webview::clear_history() {
        webview -> call_deferred("clear_all_browsing_data");
    }

    void Webview::eval(const std::string& input) {
        webview -> call_deferred("eval", to_godot_string(input));
    }

    void Webview::focus() {
        webview -> call_deferred("focus");
    }

    void Webview::reload() {
        webview -> call_deferred("reload");
    }

    void Webview::update() {
        webview -> call_deferred("resize");
        webview -> call_deferred("update_visibility");
    }

    bool Webview::is_visible() {
        return (bool)webview -> call("is_visible");
    }

    void Webview::set_visible(bool state) {
        webview -> call_deferred("set_visible", state);
    }

    bool Webview::is_fullscreen() {
        return (bool)webview -> get("full_window_size");
    }

    void Webview::set_fullscreen(bool state) {
        webview -> set_deferred("full_window_size", state);
    }

    bool Webview::is_devtools_visible() {
        return (bool)webview -> call("is_devtools_open");
    }

    void Webview::set_devtools_visible(bool state) {
        if (state) webview -> call_deferred("open_devtools");
        else webview -> call_deferred("close_devtools");
    }

    void Webview::emit(const std::string& input) {
        webview -> call_deferred("post_message", to_godot_string(input));
    }
}
#endif