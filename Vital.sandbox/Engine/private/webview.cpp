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
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/webview.h>
#include <Vital.sandbox/Manager/public/kit.h>


/////////////////////////////
// Vital: Engine: Webview //
/////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Webview::Webview(const Options& options) {
        this -> options = options;
        instances.push_back(this);

        godot::Object* object = godot::ClassDB::instantiate("WebView");
        if (!object) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: webview plugin missing");
        else webview = godot::Object::cast_to<godot::Control>(object);
        webview -> set("full_window_size", options.fullscreen);
        webview -> set("transparent", options.transparent);
        webview -> set("incognito", options.incognito);
        webview -> set("autoplay", options.autoplay);
        webview -> set("zoom_hotkeys", options.zoomable);
        webview -> set("forward_input_events", false);

        Engine::Core::get_singleton() -> enqueue([this]() {
            Engine::Canvas::get_singleton() -> add_child(webview);
            webview -> connect("ipc_message", godot::Callable(this, "on_message"));
            webview -> connect("page_load_finished", godot::Callable(this, "on_page_loaded"));
            load_url("https://github.com/ov-studio/Vital.sandbox");
            set_visible(false);
            set_devtools_visible(false);
        });
    }

    Webview::~Webview() {
        yield_forwarder();
        instances.erase(std::remove(instances.begin(), instances.end(), this), instances.end());
        fill_forwarder_vacancy();

        if (!webview) return;
        webview -> queue_free();
        webview = nullptr;
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
        return webview -> get("full_window_size");
    }

    bool Webview::is_transparent() {
        return webview -> get("transparent");
    }

    bool Webview::is_incognito() {
        return webview -> get("incognito");
    }

    bool Webview::is_autoplay() {
        return webview -> get("autoplay");
    }

    bool Webview::is_zoomable() {
        return webview -> get("zoom_hotkeys");
    }

    bool Webview::is_devtools_visible() {
        return webview -> call("is_devtools_open");
    }

    bool Webview::is_forward_input() {
        return options.forward_input;
    }


    // Getters //
    godot::Vector2 Webview::get_position() {
        return webview -> get_position();
    }

    godot::Vector2 Webview::get_size() {
        return webview -> get_size();
    }


    // Setters //
    void Webview::set_visible(bool state) {
        webview -> set_visible(state);
        if (state) fill_forwarder_vacancy();
        else if (input_forwarder == this) {
            yield_forwarder();
            fill_forwarder_vacancy();
        }
    }

    void Webview::set_focussed(bool state) {
        if (state) {
            if (!is_visible()) return;
            if (!options.forward_input) return; // Flagged out of ever forwarding input - explicit requests can't override this.
            if (input_forwarder == this) return;
            if (input_forwarder) input_forwarder -> yield_forwarder();
            input_forwarder = this;
            eval("window.vsdk_forward_input = true;");
            webview -> call_deferred("focus");
        }
        else {
            if (input_forwarder != this) return;
            yield_forwarder();
            fill_forwarder_vacancy();
        }
    }

    Webview* Webview::select_fallback_forwarder() {
        std::vector<Webview*> candidates;
        for (Webview* instance : instances) {
            if (!instance -> options.forward_input) continue;
            if (!instance -> is_visible()) continue;
            candidates.push_back(instance);
        }
        if (candidates.empty()) return nullptr;

        std::vector<Webview*> fullscreen_candidates;
        for (Webview* instance : candidates) {
            if (instance -> is_fullscreen()) fullscreen_candidates.push_back(instance);
        }

        std::vector<Webview*> pool;
        if (!fullscreen_candidates.empty()) pool = fullscreen_candidates;
        else {
            float best_area = -1.0f;
            for (Webview* instance : candidates) {
                godot::Vector2 size = instance -> get_size();
                float area = size.x * size.y;
                if (area > best_area) {
                    best_area = area;
                    pool.clear();
                    pool.push_back(instance);
                }
                else if (area == best_area) pool.push_back(instance);
            }
        }
        static std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<size_t> dist(0, pool.size() - 1);
        return pool[dist(rng)];
    }

    void Webview::fill_forwarder_vacancy() {
        if (input_forwarder != nullptr) return; // Never contest an existing forwarder.
        Webview* fallback = select_fallback_forwarder();
        if (fallback) fallback -> set_focussed(true);
    }

    void Webview::set_position(const godot::Vector2& position) {
        webview -> set_position(position);
    }

    void Webview::set_size(const godot::Vector2& size) {
        webview -> set_size(size);
        fill_forwarder_vacancy(); // TODO: Use resize callback instead??
    }

    void Webview::set_devtools_visible(bool state) {
        if (state) webview -> call_deferred("open_devtools");
        else webview -> call_deferred("close_devtools");
    }

    void Webview::set_message_handler(std::function<void(godot::String)> handler) {
        message_handler = std::move(handler);
    }

    void Webview::reset_message_handler() {
        message_handler = nullptr;
    }


    // Misc //
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

    void Webview::on_page_loaded(godot::String url) {
        std::ostringstream js;
        js << "(function() {";
        for (const auto& [src, content] : Manager::Kit::fetch_modules("js")) js << content << "\n";
        js << "})();";
        eval(js.str());
        if (input_forwarder == this) eval("window.vsdk_forward_input = true;");
    }
}
#endif