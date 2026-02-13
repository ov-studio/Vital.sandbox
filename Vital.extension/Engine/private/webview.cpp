/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: webview.cpp
     Author: ov-studio
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
        webview -> connect("ipc_message", godot::Callable(this, "on_message"));
        webview -> call_deferred("load_url", "https://github.com/ov-studio/Vital.sandbox");

    }

    Webview::~Webview() {
        if (!webview) return;
        webview -> queue_free();
        webview = nullptr;
    }

    void Webview::_bind_methods() {
        godot::ClassDB::bind_method(godot::D_METHOD("on_message", "message"), &Webview::on_message);
    }


    // Getters //
    bool Webview::is_visible() {
        return (bool)webview -> call("is_visible");
    }

    bool Webview::is_fullscreen() {
        return (bool)webview -> call("get_full_window_size");
    }

    bool Webview::is_transparent() {
        return (bool)webview -> call("get_transparent");
    }

    bool Webview::is_autoplay() {
        return (bool)webview -> call("get_autoplay");
    }

    bool Webview::is_zoomable() {
        return (bool)webview -> call("get_zoom_hotkeys");
    }

    bool Webview::is_devtools_visible() {
        return (bool)webview -> call("is_devtools_open");
    }

    godot::Vector2 Webview::get_position() {
        return webview -> get_position();
    }

    godot::Vector2 Webview::get_size() {
        return webview -> get_size();
    }


    // Setters //
    void Webview::set_visible(bool state) {
        webview -> call_deferred("set_visible", state);
    }

    void Webview::set_fullscreen(bool state) {
        webview -> call("set_full_window_size", state);
    }

    void Webview::set_transparent(bool state) {
        webview -> call("set_transparent", state);
    }

    void Webview::set_autoplay(bool state) {
        webview -> call("set_autoplay", state);
    }

    void Webview::set_zoomable(bool state) {
        webview -> call("set_zoom_hotkeys", state);
    }

    void Webview::set_devtools_visible(bool state) {
        if (state) webview -> call_deferred("open_devtools");
        else webview -> call_deferred("close_devtools");
    }

    void Webview::set_position(const godot::Vector2& position) {
        webview -> set_position(position);
    }

    void Webview::set_size(const godot::Vector2& size) {
        webview -> set_size(size);
    }

    void Webview::set_message_handler(std::function<void(godot::String)> handler) {
        message_handler = std::move(handler);
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

    void Webview::focus() {
        webview -> call_deferred("focus");
    }

    void Webview::reload() {
        webview -> call_deferred("reload");
    }

    void Webview::zoom(float value) {
        webview -> call_deferred("zoom", value);
    }

    void Webview::update() {
        webview -> call_deferred("resize");
        webview -> call_deferred("update_visibility");
    }

    void Webview::eval(const std::string& input) {
        webview -> call_deferred("eval", to_godot_string(input));
    }

    void Webview::emit(const std::string& input) {
        webview -> call_deferred("post_message", to_godot_string(input));
    }


    // Events //
    void Webview::on_message(godot::String message) {
        godot::UtilityFunctions::print("IPC Message: ", message);
    }
}
#endif