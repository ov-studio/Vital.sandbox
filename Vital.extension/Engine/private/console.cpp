/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: console.cpp
     Author: vStudio
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
        webview = new Vital::Godot::Webview;
        webview -> set_position({0, 0});
        webview -> set_size({850, 425});
        webview -> set_visible(true);
        webview -> set_fullscreen(false);
        webview -> set_transparent(true);
        webview -> set_autoplay(false);
        webview -> set_zoomable(false);
        webview -> set_devtools_visible(false);
        webview -> load_from_raw(Vital::Tool::File::read_text(to_godot_string(get_directory()), "console.html"));
    }

    Console::~Console() {
        if (!webview) return;
        delete webview;
        webview = nullptr;
    }


    // Getters //
    Console* Console::get_singleton() {
        if (!singleton) {
            singleton = memnew(Console);
            // TODO: Causes crashs??
            //singleton -> update_size();
        }
        return singleton;
    }

    void Console::update_size() {
        godot::UtilityFunctions::print("updated console");
        auto vp_size = get_viewport() -> get_visible_rect().size;
        float max_width  = vp_size.x * 0.75f;
        float max_height = vp_size.y * 0.75f;
        webview -> set_size({
            godot::Math::min(850.0f, max_width),
            godot::Math::min(425.0f, max_height)
        });
    }
}
#endif