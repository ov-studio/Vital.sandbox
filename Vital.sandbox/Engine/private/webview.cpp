/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: webview.cpp
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
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/webview.h>


/////////////////////////////
// Vital: Engine: Webview //
/////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Webview::Webview(const Options& options) {
        godot::Object* object = godot::ClassDB::instantiate("WebView");
        if (!object) throw Tool::Log::fetch("webview-failed", Tool::Log::Type::Error, "No compatible plugin found");
        else {
            webview = godot::Object::cast_to<godot::Control>(object);
            if (!webview) {
                memdelete(object);
                throw Tool::Log::fetch("webview-failed", Tool::Log::Type::Error, "No compatible device found");
            }
        }
        webview -> set("full_window_size", options.fullscreen);
        webview -> set("transparent", options.transparent);
        webview -> set("incognito", options.incognito);
        webview -> set("autoplay", options.autoplay);
        webview -> set("zoom_hotkeys", options.zoomable);
        Core::get_singleton() -> push_deferred([this]() {
            Canvas::get_singleton() -> add_child(webview);
            webview -> connect("ipc_message", godot::Callable(this, "on_message"));
            load_url("https://github.com/ov-studio/Vital.sandbox");
            set_visible(false);
            set_devtools_visible(false);
        });
    }

    Webview::~Webview() {
        if (!webview) return;
        webview -> queue_free();
        webview = nullptr;
    }

    void Webview::_bind_methods() {
        godot::ClassDB::bind_method(godot::D_METHOD("on_message", "message"), &Webview::on_message);
    }


    // Managers //
    Webview* Webview::create(const Options& options) {
        return memnew(Webview(options));
    }

    void Webview::destroy() {
        memdelete(this);
    }


    // Checkers //
    bool Webview::is_visible() {
        return webview -> is_visible();
    }

    bool Webview::is_fullscreen() {
        return (bool)webview -> get("full_window_size");
    }

    bool Webview::is_transparent() {
        return (bool)webview -> get("transparent");
    }

    bool Webview::is_incognito() {
        return (bool)webview -> get("incognito");
    }

    bool Webview::is_autoplay() {
        return (bool)webview -> get("autoplay");
    }

    bool Webview::is_zoomable() {
        return (bool)webview -> get("zoom_hotkeys");
    }

    bool Webview::is_devtools_visible() {
        return (bool)webview -> call("is_devtools_open");
    }


    // Setters //
    void Webview::set_visible(bool state) {
        webview -> set_visible(state);
        webview -> call("focus_parent");
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


    // Getters //
    godot::Vector2 Webview::get_position() {
        return webview -> get_position();
    }

    godot::Vector2 Webview::get_size() {
        return webview -> get_size();
    }


    // APIs //
    void Webview::load_url(const std::string& url) {
        webview -> call_deferred("load_url", Tool::to_godot_string(url));
    }

    void Webview::load_html(const std::string& raw) {
        webview -> call_deferred("load_html", Tool::to_godot_string(raw));
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
        webview -> call_deferred("eval", Tool::to_godot_string(input));
    }

    void Webview::emit(const std::string& input) {
        webview -> call_deferred("post_message", Tool::to_godot_string(input));
    }


    // Events //
    void Webview::on_message(godot::String message) {
        if (!message_handler) return;
        message_handler(message);
    }
}
#endif