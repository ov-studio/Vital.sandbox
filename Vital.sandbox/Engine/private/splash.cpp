/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: splash.cpp
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
#include <Vital.sandbox/Engine/public/splash.h>
#include <Vital.sandbox/Manager/public/kit.h>
#include <Vital.sandbox/API/utility/input.h>


////////////////////////////
// Vital: Engine: Splash //
////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Splash::Splash() {
        blackcover = memnew(godot::ColorRect);
        blackcover -> set_anchors_preset(godot::Control::PRESET_FULL_RECT);
        blackcover -> set_color(godot::Color(0, 0, 0, 1));
        blackcover -> set_as_top_level(true);
        Engine::Core::get_singleton() -> add_child(blackcover);

        Tool::Event::bind("kit:ready", [this](Tool::Stack) {
            Engine::Core::get_singleton() -> enqueue([this]() {
                show();
            });
        });
    }

    Splash::~Splash() {
        if (blackcover) {
            blackcover -> queue_free();
            blackcover = nullptr;
        }
        if (webview) {
            webview -> destroy();
            webview = nullptr;
        }
    }


    // Managers //
    bool Splash::is_ready() {
        return webview_ready.load();
    }

    bool Splash::is_visible() {
        return webview ? webview -> is_visible() : false;
    }

    void Splash::ready() {
        webview_ready.store(true);
        rapidjson::Document reply;
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        reply.SetObject();
        reply.AddMember("action", "init", reply.GetAllocator());
        reply.Accept(writer);
        webview -> emit(buffer.GetString());
        if (blackcover) {
            blackcover -> queue_free();
            blackcover = nullptr;
        }
    }

    void Splash::show() {
        Engine::Webview::Options options;
        options.z_index = Engine::Webview::system_z_floor + 2;
        options.fullscreen = true;
        options.transparent = true;
        options.incognito = true;
        options.autoplay = false;
        options.zoomable = false;
        options.forward_input = false;
        options.overlay = false;
        webview = Engine::Webview::create(options);
        webview -> set_position({0, 0});
        webview -> load_url(Engine::Core::get_singleton() -> get_http_url("cache/Vital.kit/splash/build/index.html"));
        webview -> set_visible(true);
        Sandbox::API::Input::push_sandbox_ui_visible();
        webview -> set_handler("message", [this](Engine::Webview::Payload payload) {
            if (auto* content = std::get_if<std::string>(&payload)) on_message(Tool::to_godot_string(*content));
        });
    }

    void Splash::hide() {
        Sandbox::API::Input::pop_sandbox_ui_visible();
        free_singleton();
    }


    // Events //
    void Splash::on_message(godot::String message) {
        rapidjson::Document document;
        document.Parse(Tool::to_std_string(message).c_str());
        if (document.HasParseError() || !document.HasMember("action")) return;
        std::string action = document["action"].GetString();
        if (action == "ready") ready();
        else if (action == "hide") hide();
    }
}
#endif
