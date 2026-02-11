/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: console.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Console Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#if defined(Vital_SDK_Client)
#pragma once
#include <Vital.extension/Engine/public/console.h>
#include <Vital.extension/Engine/public/core.h>
#include <Vital.extension/Engine/public/canvas.h>


////////////////////////////
// Vital: Godot: Console //
////////////////////////////

namespace Vital::Godot {
    // Instantiators //
    Console::Console() {
        webview = memnew(Vital::Godot::Webview);
        webview -> set_position({0, 0});
        webview -> set_size({850, 425});
        webview -> set_visible(true);
        webview -> set_fullscreen(true);
        webview -> set_transparent(true);
        webview -> set_autoplay(false);
        webview -> set_zoomable(false);
        webview -> set_devtools_visible(false);
        webview -> load_from_raw(Vital::Tool::File::read_text(to_godot_string(get_directory()), "console.html"));
    }

    Console::~Console() {
        if (!webview) return;
        memdelete(webview);
        webview = nullptr;
    }


    // Getters //
    Console* Console::get_singleton() {
        if (!singleton) singleton = memnew(Console);
        return singleton;
    }

    void Console::free_singleton() {
        if (!singleton) return;
        memdelete(singleton);
        singleton = nullptr;
    }
}
#endif