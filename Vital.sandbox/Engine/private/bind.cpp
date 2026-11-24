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

namespace Vital::Engine {
    // Helpers //
    int Bind::to_virtual_key(godot::Key key) {
        const int code = static_cast<int>(key);
        #if defined(VSDK_WINDOWS)
            if (code >= static_cast<int>(godot::Key::KEY_F1) && code <= static_cast<int>(godot::Key::KEY_F24)) return VK_F1 + (code - static_cast<int>(godot::Key::KEY_F1));
            if (code >= static_cast<int>(godot::Key::KEY_KP_0) && code <= static_cast<int>(godot::Key::KEY_KP_9)) return VK_NUMPAD0 + (code - static_cast<int>(godot::Key::KEY_KP_0));
            if ((code >= 'A' && code <= 'Z') || (code >= '0' && code <= '9')) return code;
            switch (key) {
                case godot::Key::KEY_ESCAPE: return VK_ESCAPE;
                case godot::Key::KEY_TAB: return VK_TAB;
                case godot::Key::KEY_BACKSPACE: return VK_BACK;
                case godot::Key::KEY_ENTER: return VK_RETURN;
                case godot::Key::KEY_KP_ENTER: return VK_RETURN;
                case godot::Key::KEY_SPACE: return VK_SPACE;
                case godot::Key::KEY_INSERT: return VK_INSERT;
                case godot::Key::KEY_DELETE: return VK_DELETE;
                case godot::Key::KEY_PAUSE: return VK_PAUSE;
                case godot::Key::KEY_PRINT: return VK_SNAPSHOT;
                case godot::Key::KEY_CLEAR: return VK_CLEAR;
                case godot::Key::KEY_HELP: return VK_HELP;
                case godot::Key::KEY_MENU: return VK_APPS;
                case godot::Key::KEY_HOME: return VK_HOME;
                case godot::Key::KEY_END: return VK_END;
                case godot::Key::KEY_LEFT: return VK_LEFT;
                case godot::Key::KEY_UP: return VK_UP;
                case godot::Key::KEY_RIGHT: return VK_RIGHT;
                case godot::Key::KEY_DOWN: return VK_DOWN;
                case godot::Key::KEY_PAGEUP: return VK_PRIOR;
                case godot::Key::KEY_PAGEDOWN: return VK_NEXT;
                case godot::Key::KEY_SHIFT: return VK_SHIFT;
                case godot::Key::KEY_CTRL: return VK_CONTROL;
                case godot::Key::KEY_ALT: return VK_MENU;
                case godot::Key::KEY_META: return VK_LWIN;
                case godot::Key::KEY_CAPSLOCK: return VK_CAPITAL;
                case godot::Key::KEY_NUMLOCK: return VK_NUMLOCK;
                case godot::Key::KEY_SCROLLLOCK: return VK_SCROLL;
                case godot::Key::KEY_KP_MULTIPLY: return VK_MULTIPLY;
                case godot::Key::KEY_KP_ADD: return VK_ADD;
                case godot::Key::KEY_KP_SUBTRACT: return VK_SUBTRACT;
                case godot::Key::KEY_KP_PERIOD: return VK_DECIMAL;
                case godot::Key::KEY_KP_DIVIDE: return VK_DIVIDE;
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
                case godot::Key::KEY_BACK: return VK_BROWSER_BACK;
                case godot::Key::KEY_FORWARD: return VK_BROWSER_FORWARD;
                case godot::Key::KEY_REFRESH: return VK_BROWSER_REFRESH;
                case godot::Key::KEY_STOP: return VK_BROWSER_STOP;
                case godot::Key::KEY_SEARCH: return VK_BROWSER_SEARCH;
                case godot::Key::KEY_FAVORITES: return VK_BROWSER_FAVORITES;
                case godot::Key::KEY_HOMEPAGE: return VK_BROWSER_HOME;
                case godot::Key::KEY_VOLUMEDOWN: return VK_VOLUME_DOWN;
                case godot::Key::KEY_VOLUMEMUTE: return VK_VOLUME_MUTE;
                case godot::Key::KEY_VOLUMEUP: return VK_VOLUME_UP;
                case godot::Key::KEY_MEDIAPLAY: return VK_MEDIA_PLAY_PAUSE;
                case godot::Key::KEY_MEDIASTOP: return VK_MEDIA_STOP;
                case godot::Key::KEY_MEDIAPREVIOUS: return VK_MEDIA_PREV_TRACK;
                case godot::Key::KEY_MEDIANEXT: return VK_MEDIA_NEXT_TRACK;
                default: return 0;
            }
        #elif defined(VSDK_MACOS)
            static constexpr int function_keys[] = { 122, 120, 99, 118, 96, 97, 98, 100, 101, 109, 103, 111, 105, 107, 113, 106, 64, 79, 80, 90 };
            static constexpr int keypad_keys[] = { 82, 83, 84, 85, 86, 87, 88, 89, 91, 92 };
            if (code >= static_cast<int>(godot::Key::KEY_F1) && code <= static_cast<int>(godot::Key::KEY_F20)) return function_keys[code - static_cast<int>(godot::Key::KEY_F1)];
            if (code >= static_cast<int>(godot::Key::KEY_KP_0) && code <= static_cast<int>(godot::Key::KEY_KP_9)) return keypad_keys[code - static_cast<int>(godot::Key::KEY_KP_0)];
            switch (key) {
                case godot::Key::KEY_ESCAPE: return 53;
                case godot::Key::KEY_TAB: return 48;
                case godot::Key::KEY_BACKSPACE: return 51;
                case godot::Key::KEY_ENTER: return 36;
                case godot::Key::KEY_KP_ENTER: return 76;
                case godot::Key::KEY_SPACE: return 49;
                case godot::Key::KEY_INSERT: return 114;
                case godot::Key::KEY_DELETE: return 117;
                case godot::Key::KEY_HOME: return 115;
                case godot::Key::KEY_END: return 119;
                case godot::Key::KEY_PAGEUP: return 116;
                case godot::Key::KEY_PAGEDOWN: return 121;
                case godot::Key::KEY_LEFT: return 123;
                case godot::Key::KEY_RIGHT: return 124;
                case godot::Key::KEY_DOWN: return 125;
                case godot::Key::KEY_UP: return 126;
                case godot::Key::KEY_SHIFT: return 56;
                case godot::Key::KEY_CTRL: return 59;
                case godot::Key::KEY_ALT: return 58;
                case godot::Key::KEY_META: return 55;
                case godot::Key::KEY_CAPSLOCK: return 57;
                case godot::Key::KEY_KP_PERIOD: return 65;
                case godot::Key::KEY_KP_MULTIPLY: return 67;
                case godot::Key::KEY_KP_ADD: return 69;
                case godot::Key::KEY_KP_DIVIDE: return 75;
                case godot::Key::KEY_KP_SUBTRACT: return 78;
                default: return 0;
            }
        #elif defined(VSDK_LINUX)
            if (code >= static_cast<int>(godot::Key::KEY_F1) && code <= static_cast<int>(godot::Key::KEY_F35)) return 0xffbe + (code - static_cast<int>(godot::Key::KEY_F1));
            if (code >= static_cast<int>(godot::Key::KEY_KP_0) && code <= static_cast<int>(godot::Key::KEY_KP_9)) return 0xffb0 + (code - static_cast<int>(godot::Key::KEY_KP_0));
            if (code >= 'A' && code <= 'Z') return code + ('a' - 'A');
            if (code >= '0' && code <= '9') return code;
            switch (key) {
                case godot::Key::KEY_ESCAPE: return 0xff1b;
                case godot::Key::KEY_TAB: return 0xff09;
                case godot::Key::KEY_BACKSPACE: return 0xff08;
                case godot::Key::KEY_ENTER: return 0xff0d;
                case godot::Key::KEY_KP_ENTER: return 0xff8d;
                case godot::Key::KEY_SPACE: return 0x20;
                case godot::Key::KEY_INSERT: return 0xff63;
                case godot::Key::KEY_DELETE: return 0xffff;
                case godot::Key::KEY_PAUSE: return 0xff13;
                case godot::Key::KEY_PRINT: return 0xff61;
                case godot::Key::KEY_MENU: return 0xff67;
                case godot::Key::KEY_HOME: return 0xff50;
                case godot::Key::KEY_LEFT: return 0xff51;
                case godot::Key::KEY_UP: return 0xff52;
                case godot::Key::KEY_RIGHT: return 0xff53;
                case godot::Key::KEY_DOWN: return 0xff54;
                case godot::Key::KEY_PAGEUP: return 0xff55;
                case godot::Key::KEY_PAGEDOWN: return 0xff56;
                case godot::Key::KEY_END: return 0xff57;
                case godot::Key::KEY_SHIFT: return 0xffe1;
                case godot::Key::KEY_CTRL: return 0xffe3;
                case godot::Key::KEY_ALT: return 0xffe9;
                case godot::Key::KEY_CAPSLOCK: return 0xffe5;
                case godot::Key::KEY_NUMLOCK: return 0xff7f;
                case godot::Key::KEY_SCROLLLOCK: return 0xff14;
                case godot::Key::KEY_KP_MULTIPLY: return 0xffaa;
                case godot::Key::KEY_KP_ADD: return 0xffab;
                case godot::Key::KEY_KP_SUBTRACT: return 0xffad;
                case godot::Key::KEY_KP_PERIOD: return 0xffae;
                case godot::Key::KEY_KP_DIVIDE: return 0xffaf;
                case godot::Key::KEY_QUOTELEFT: return 0x60;
                case godot::Key::KEY_MINUS: return 0x2d;
                case godot::Key::KEY_EQUAL: return 0x3d;
                case godot::Key::KEY_BRACKETLEFT: return 0x5b;
                case godot::Key::KEY_BACKSLASH: return 0x5c;
                case godot::Key::KEY_BRACKETRIGHT: return 0x5d;
                case godot::Key::KEY_SEMICOLON: return 0x3b;
                case godot::Key::KEY_APOSTROPHE: return 0x27;
                case godot::Key::KEY_COMMA: return 0x2c;
                case godot::Key::KEY_PERIOD: return 0x2e;
                case godot::Key::KEY_SLASH: return 0x2f;
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
        #elif defined(VSDK_MACOS)
            return CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState, static_cast<CGKeyCode>(code)) ? 1 : 0;
        #elif defined(VSDK_LINUX)
            return get_x11_key_state(code);
        #else
            return -1;
        #endif
    }
    
    int Bind::get_x11_key_state(int keysym) {
        #if defined(VSDK_LINUX)
            struct X11 {
                void* display = nullptr;
                int (*query_keymap)(void*, char*) = nullptr;
                unsigned char (*keysym_to_keycode)(void*, unsigned long) = nullptr;
            };
            static X11 x11;
            static bool attempted = false;
            if (!attempted) {
                attempted = true;
                if (void* lib = dlopen("libX11.so.6", RTLD_LAZY)) {
                    auto open_display = reinterpret_cast<void* (*)(const char*)>(dlsym(lib, "XOpenDisplay"));
                    auto query_keymap = reinterpret_cast<int (*)(void*, char*)>(dlsym(lib, "XQueryKeymap"));
                    auto keysym_to_keycode = reinterpret_cast<unsigned char (*)(void*, unsigned long)>(dlsym(lib, "XKeysymToKeycode"));
                    if (open_display && query_keymap && keysym_to_keycode) {
                        x11.query_keymap = query_keymap;
                        x11.keysym_to_keycode = keysym_to_keycode;
                        x11.display = open_display(nullptr);
                    }
                }
            }
            if (!x11.display) return -1;
            auto display_server = godot::DisplayServer::get_singleton();
            if (!display_server || display_server -> get_name() != godot::String("X11")) return -1;
            const unsigned char keycode = x11.keysym_to_keycode(x11.display, static_cast<unsigned long>(keysym));
            if (!keycode) return -1;
            char keymap[32] = {};
            x11.query_keymap(x11.display, keymap);
            return (keymap[keycode / 8] >> (keycode % 8)) & 1;
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
        #elif defined(VSDK_MACOS) || defined(VSDK_LINUX)
            auto display_server = godot::DisplayServer::get_singleton();
            return display_server && display_server -> window_is_focused();
        #else
            return true;
        #endif
    }

    bool Bind::is_available(const Slot& slot) {
        if (slot.key == godot::Key::KEY_NONE) return false;
        switch (slot.type) {
            case Type::MainMenu: return Engine::MainMenu::has_singleton() && Manager::Network::get_singleton() -> is_connected();
            case Type::Screenshot: return Manager::Network::get_singleton() -> is_connected();
            default: return true;
        }
    }

    Bind::Slot* Bind::find(godot::Key keycode) {
        for (auto& slot : slots) {
            if (slot.key == keycode && is_available(slot)) return &slot;
        }
        return nullptr;
    }

    void Bind::refresh() {
        for (auto& slot : slots) {
            const auto bind = Manager::Kit::get_singleton() -> get_json_value(slot.config, "bind");
            slot.key = bind.is<std::string>() ? godot::OS::get_singleton() -> find_keycode_from_string(Tool::to_godot_string(bind.as<std::string>())) : godot::Key::KEY_NONE;
        }
    }

    void Bind::run(const Slot& slot) {
        switch (slot.type) {
            case Type::Console: {
                Engine::Console::get_singleton() -> toggle();
                break;
            }
            case Type::MainMenu: {
                Engine::MainMenu::get_singleton() -> toggle();
                break;
            }
            case Type::Screenshot: {
                auto path = fmt::format("{}.png", Tool::get_timestamp_tag());
                Engine::Display::get_singleton() -> screenshot(Tool::get_directory("screenshots"), path);
                log("sbox", fmt::format("screenshot saved to `screenshots/{}`", path));
                break;
            }
        }
    }


    // Managers //
    bool Bind::handle_key(godot::InputEventKey* event) {
        if (!event -> is_pressed()) return false;
        const auto keycode = event -> get_keycode();
        auto slot = find(keycode);
        if (!slot) return false;
        const int state = get_key_state(keycode);
        if (state < 0) run(*slot);
        else if (state == 1 && !slot -> latched) {
            slot -> latched = true;
            run(*slot);
        }
        return true;
    }

    void Bind::poll(double delta) {
        if (!Manager::Kit::get_singleton()) return;
        refresh_timer -= delta;
        if (refresh_timer <= 0.0) {
            refresh_timer = 1.0;
            refresh();
        }
        const bool foreground = is_game_foreground();
        for (auto& slot : slots) {
            const int state = get_key_state(slot.key);
            if (state < 0) continue;
            if (state == 0) slot.latched = false;
            else if (!foreground) slot.latched = true;
            else if (!slot.latched) {
                slot.latched = true;
                if (is_available(slot)) run(slot);
            }
        }
    }
}
#endif
