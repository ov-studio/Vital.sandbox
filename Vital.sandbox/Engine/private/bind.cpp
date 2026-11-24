/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: bind.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 24/11/2026
     Desc: Bind Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/bind.h>
#include <Vital.sandbox/Engine/public/console.h>
#include <Vital.sandbox/Engine/public/display.h>
#include <Vital.sandbox/Engine/public/mainmenu.h>
#include <Vital.sandbox/Manager/public/network.h>
#include <Vital.sandbox/Manager/public/kit.h>


//////////////////////////
// Vital: Engine: Bind //
//////////////////////////

// TODO: Improve
namespace Vital::Engine {
    // Helpers //
    int Bind::to_virtual_key(godot::Key key) {
        #if defined(VSDK_WINDOWS)
            const int code = static_cast<int>(key);
            if (code >= static_cast<int>(godot::Key::KEY_F1) && code <= static_cast<int>(godot::Key::KEY_F24)) return VK_F1 + (code - static_cast<int>(godot::Key::KEY_F1));
            if ((code >= 'A' && code <= 'Z') || (code >= '0' && code <= '9')) return code;
            switch (key) {
                case godot::Key::KEY_ESCAPE: return VK_ESCAPE;
                case godot::Key::KEY_TAB: return VK_TAB;
                case godot::Key::KEY_BACKSPACE: return VK_BACK;
                case godot::Key::KEY_ENTER: return VK_RETURN;
                case godot::Key::KEY_SPACE: return VK_SPACE;
                case godot::Key::KEY_INSERT: return VK_INSERT;
                case godot::Key::KEY_DELETE: return VK_DELETE;
                case godot::Key::KEY_PAUSE: return VK_PAUSE;
                case godot::Key::KEY_PRINT: return VK_SNAPSHOT;
                case godot::Key::KEY_HOME: return VK_HOME;
                case godot::Key::KEY_END: return VK_END;
                case godot::Key::KEY_LEFT: return VK_LEFT;
                case godot::Key::KEY_UP: return VK_UP;
                case godot::Key::KEY_RIGHT: return VK_RIGHT;
                case godot::Key::KEY_DOWN: return VK_DOWN;
                case godot::Key::KEY_PAGEUP: return VK_PRIOR;
                case godot::Key::KEY_PAGEDOWN: return VK_NEXT;
                case godot::Key::KEY_QUOTELEFT: return VK_OEM_3;
                case godot::Key::KEY_MINUS: return VK_OEM_MINUS;
                case godot::Key::KEY_EQUAL: return VK_OEM_PLUS;
                case godot::Key::KEY_BRACKETLEFT: return VK_OEM_4;
                case godot::Key::KEY_BRACKETRIGHT: return VK_OEM_6;
                case godot::Key::KEY_BACKSLASH: return VK_OEM_5;
                case godot::Key::KEY_SEMICOLON: return VK_OEM_1;
                case godot::Key::KEY_APOSTROPHE: return VK_OEM_7;
                case godot::Key::KEY_COMMA: return VK_OEM_COMMA;
                case godot::Key::KEY_PERIOD: return VK_OEM_PERIOD;
                case godot::Key::KEY_SLASH: return VK_OEM_2;
                default: return 0;
            }
        #else
            return 0;
        #endif
    }

    int Bind::get_key_state(godot::Key key) {
        const int code = to_virtual_key(key);
        if (!code) return -1;
        #if defined(VSDK_WINDOWS)
            return (GetAsyncKeyState(code) & 0x8000) ? 1 : 0;
        #else
            return -1;
        #endif
    }

    bool Bind::is_game_foreground() {
        #if defined(VSDK_WINDOWS)
            HWND window = GetForegroundWindow();
            if (!window) return false;
            DWORD process = 0;
            GetWindowThreadProcessId(window, &process);
            return process == GetCurrentProcessId();
        #else
            return true;
        #endif
    }

    void Bind::refresh() {
        static constexpr const char* configs[count] = { "config/console", "config/mainmenu", "config/screenshot" };
        for (int i = 0; i < count; ++i) {
            const auto bind = Manager::Kit::get_singleton() -> get_json_value(configs[i], "bind");
            keys[i] = bind.is<std::string>() ? godot::OS::get_singleton() -> find_keycode_from_string(Tool::to_godot_string(bind.as<std::string>())) : godot::Key::KEY_NONE;
        }
    }

    bool Bind::is_available(int type) {
        if (keys[type] == godot::Key::KEY_NONE) return false;
        if (type == mainmenu) return Engine::MainMenu::has_singleton() && Manager::Network::get_singleton() -> is_connected();
        if (type == screenshot) return Manager::Network::get_singleton() -> is_connected();
        return true;
    }

    int Bind::find(godot::Key keycode) {
        for (int i = 0; i < count; ++i) {
            if (keycode == keys[i] && is_available(i)) return i;
        }
        return -1;
    }

    void Bind::run(int type) {
        if (type == console) Engine::Console::get_singleton() -> toggle();
        else if (type == mainmenu) Engine::MainMenu::get_singleton() -> toggle();
        else if (type == screenshot) {
            auto path = fmt::format("{}.png", Tool::get_timestamp_tag());
            Engine::Display::get_singleton() -> screenshot(Tool::get_directory("screenshots"), path);
            log("sbox", fmt::format("screenshot saved to `screenshots/{}`", path));
        }
    }


    // Managers //
    bool Bind::handle_key(godot::InputEventKey* event) {
        refresh();
        // Releases are ignored on purpose: Godot also sends one when the keyboard focus moves into a webview (the console that just opened)
        // while the key is still down, which would unlatch the bind and make the poll toggle it a second time
        if (!event -> is_pressed()) return false;
        const auto keycode = event -> get_keycode();
        const int type = find(keycode);
        if (type < 0) return false;
        // The poll consumes the press first (and only it unlatches, once the key is really up). The page forwards the same key later,
        // after a frame hitch even once it is released, so a copy is only acted on while the key is really down and not consumed yet.
        // A key that can't be read from the keyboard (state < 0, e.g. not on Windows) is acted on directly
        const int state = get_key_state(keycode);
        if (state < 0) run(type);
        else if (state == 1 && !latched[type]) {
            latched[type] = true;
            run(type);
        }
        return true;
    }

    // Reads the binds straight from the keyboard every frame (a no-op where the keys can't be read, i.e. not on Windows): while a webview owns the keyboard the key reaches the game only
    // through the page, which is delayed and loses the key whenever the focus is being handed over (console / menu opening, a click)
    void Bind::poll(double delta) {
        if (!Manager::Kit::get_singleton()) return;
        refresh_timer -= delta;
        if (refresh_timer <= 0.0) {
            refresh_timer = 1.0;
            refresh();
        }
        const bool foreground = is_game_foreground();
        for (int i = 0; i < count; ++i) {
            const int state = get_key_state(keys[i]);
            if (state < 0) continue;
            const bool down = state == 1;
            if (!down) latched[i] = false;
            else if (!foreground) latched[i] = true;
            else if (!latched[i]) {
                latched[i] = true;
                if (is_available(i)) run(i);
            }
        }
    }
}
#endif
