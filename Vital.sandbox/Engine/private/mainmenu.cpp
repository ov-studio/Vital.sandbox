/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: mainmenu.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: MainMenu Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/mainmenu.h>
#include <Vital.sandbox/Manager/public/kit.h>
#include <Vital.sandbox/API/utility/input.h>


//////////////////////////////
// Vital: Engine: MainMenu //
//////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    MainMenu::MainMenu() {
        Engine::Webview::Options options;
        options.z_index = Engine::Webview::system_z_floor + 1;
        options.fullscreen = true;
        options.transparent = true;
        options.incognito = true;
        options.autoplay = false;
        options.zoomable = false;
        options.forward_input = false;
        options.overlay = false;
        webview = Engine::Webview::create(options);
        webview -> set_position({0, 0});
        webview -> load_url(Engine::Core::get_singleton() -> get_http_url("cache/Vital.kit/mainmenu/build/index.html"));
        webview -> set_handler("message", [this](Engine::Webview::Payload payload) {
            if (auto content = std::get_if<std::string>(&payload)) on_message(Tool::to_godot_string(*content));
        });
    }

    MainMenu::~MainMenu() {
        if (webview) {
            webview -> destroy();
            webview = nullptr;
        }
    }


    // Managers //
    bool MainMenu::is_ready() {
        return webview_ready.load();
    }

    bool MainMenu::is_visible() {
        return webview -> is_visible();
    }

    void MainMenu::ready() {
        webview_ready.store(true);
    }

    void MainMenu::show() {
        if (is_visible()) return;
        webview -> set_visible(true);
        Sandbox::API::Input::push_sandbox_ui_visible();
    }

    void MainMenu::hide() {
        if (!is_visible()) return;
        webview -> set_visible(false);
        Sandbox::API::Input::pop_sandbox_ui_visible();
    }


    // Events //
    void MainMenu::on_message(godot::String message) {
        rapidjson::Document document;
        document.Parse(Tool::to_std_string(message).c_str());
        if (document.HasParseError() || !document.HasMember("action")) return;
        std::string action = document["action"].GetString();
        if (action == "ready") ready();
        else if (action == "hide") hide();
    }
}
#endif
