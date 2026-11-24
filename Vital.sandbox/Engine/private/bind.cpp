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
    #if defined(VSDK_WINDOWS)
        #define NATIVE(key, win, mac, x11) { godot::Key::key, win }
    #elif defined(VSDK_MACOS)
        #define NATIVE(key, win, mac, x11) { godot::Key::key, mac }
    #elif defined(VSDK_LINUX)
        #define NATIVE(key, win, mac, x11) { godot::Key::key, x11 }
    #else
        #define NATIVE(key, win, mac, x11) { godot::Key::key, 0 }
    #endif

    int Bind::to_virtual_key(godot::Key key) {
        const int code = static_cast<int>(key);
        const int function = code - static_cast<int>(godot::Key::KEY_F1);
        const int keypad = code - static_cast<int>(godot::Key::KEY_KP_0);
        #if defined(VSDK_WINDOWS)
            if (function >= 0 && function < 24) return VK_F1 + function;
            if (keypad >= 0 && keypad < 10) return VK_NUMPAD0 + keypad;
            if ((code >= 'A' && code <= 'Z') || (code >= '0' && code <= '9')) return code;
        #elif defined(VSDK_MACOS)
            static constexpr int function_keys[] = { 122, 120, 99, 118, 96, 97, 98, 100, 101, 109, 103, 111, 105, 107, 113, 106, 64, 79, 80, 90 };
            static constexpr int keypad_keys[] = { 82, 83, 84, 85, 86, 87, 88, 89, 91, 92 };
            if (function >= 0 && function < 20) return function_keys[function];
            if (keypad >= 0 && keypad < 10) return keypad_keys[keypad];
        #elif defined(VSDK_LINUX)
            if (function >= 0 && function < 35) return 0xffbe + function;
            if (keypad >= 0 && keypad < 10) return 0xffb0 + keypad;
            if (code >= 'A' && code <= 'Z') return code + ('a' - 'A');
            if (code >= '0' && code <= '9') return code;
        #endif
        static constexpr Native natives[] = {
            NATIVE(KEY_ESCAPE,        VK_ESCAPE,            53,  0xff1b),
            NATIVE(KEY_TAB,           VK_TAB,               48,  0xff09),
            NATIVE(KEY_BACKSPACE,     VK_BACK,              51,  0xff08),
            NATIVE(KEY_ENTER,         VK_RETURN,            36,  0xff0d),
            NATIVE(KEY_KP_ENTER,      VK_RETURN,            76,  0xff8d),
            NATIVE(KEY_SPACE,         VK_SPACE,             49,  0x20),
            NATIVE(KEY_INSERT,        VK_INSERT,            114, 0xff63),
            NATIVE(KEY_DELETE,        VK_DELETE,            117, 0xffff),
            NATIVE(KEY_PAUSE,         VK_PAUSE,             0,   0xff13),
            NATIVE(KEY_PRINT,         VK_SNAPSHOT,          0,   0xff61),
            NATIVE(KEY_CLEAR,         VK_CLEAR,             0,   0),
            NATIVE(KEY_HELP,          VK_HELP,              0,   0),
            NATIVE(KEY_MENU,          VK_APPS,              0,   0xff67),
            NATIVE(KEY_HOME,          VK_HOME,              115, 0xff50),
            NATIVE(KEY_END,           VK_END,               119, 0xff57),
            NATIVE(KEY_LEFT,          VK_LEFT,              123, 0xff51),
            NATIVE(KEY_UP,            VK_UP,                126, 0xff52),
            NATIVE(KEY_RIGHT,         VK_RIGHT,             124, 0xff53),
            NATIVE(KEY_DOWN,          VK_DOWN,              125, 0xff54),
            NATIVE(KEY_PAGEUP,        VK_PRIOR,             116, 0xff55),
            NATIVE(KEY_PAGEDOWN,      VK_NEXT,              121, 0xff56),
            NATIVE(KEY_SHIFT,         VK_SHIFT,             56,  0xffe1),
            NATIVE(KEY_CTRL,          VK_CONTROL,           59,  0xffe3),
            NATIVE(KEY_ALT,           VK_MENU,              58,  0xffe9),
            NATIVE(KEY_META,          VK_LWIN,              55,  0),
            NATIVE(KEY_CAPSLOCK,      VK_CAPITAL,           57,  0xffe5),
            NATIVE(KEY_NUMLOCK,       VK_NUMLOCK,           0,   0xff7f),
            NATIVE(KEY_SCROLLLOCK,    VK_SCROLL,            0,   0xff14),
            NATIVE(KEY_KP_MULTIPLY,   VK_MULTIPLY,          67,  0xffaa),
            NATIVE(KEY_KP_ADD,        VK_ADD,               69,  0xffab),
            NATIVE(KEY_KP_SUBTRACT,   VK_SUBTRACT,          78,  0xffad),
            NATIVE(KEY_KP_PERIOD,     VK_DECIMAL,           65,  0xffae),
            NATIVE(KEY_KP_DIVIDE,     VK_DIVIDE,            75,  0xffaf),
            NATIVE(KEY_QUOTELEFT,     VK_OEM_3,             0,   0x60),
            NATIVE(KEY_MINUS,         VK_OEM_MINUS,         0,   0x2d),
            NATIVE(KEY_EQUAL,         VK_OEM_PLUS,          0,   0x3d),
            NATIVE(KEY_BRACKETLEFT,   VK_OEM_4,             0,   0x5b),
            NATIVE(KEY_BRACKETRIGHT,  VK_OEM_6,             0,   0x5d),
            NATIVE(KEY_BACKSLASH,     VK_OEM_5,             0,   0x5c),
            NATIVE(KEY_SEMICOLON,     VK_OEM_1,             0,   0x3b),
            NATIVE(KEY_APOSTROPHE,    VK_OEM_7,             0,   0x27),
            NATIVE(KEY_COMMA,         VK_OEM_COMMA,         0,   0x2c),
            NATIVE(KEY_PERIOD,        VK_OEM_PERIOD,        0,   0x2e),
            NATIVE(KEY_SLASH,         VK_OEM_2,             0,   0x2f),
            NATIVE(KEY_BACK,          VK_BROWSER_BACK,      0,   0),
            NATIVE(KEY_FORWARD,       VK_BROWSER_FORWARD,   0,   0),
            NATIVE(KEY_REFRESH,       VK_BROWSER_REFRESH,   0,   0),
            NATIVE(KEY_STOP,          VK_BROWSER_STOP,      0,   0),
            NATIVE(KEY_SEARCH,        VK_BROWSER_SEARCH,    0,   0),
            NATIVE(KEY_FAVORITES,     VK_BROWSER_FAVORITES, 0,   0),
            NATIVE(KEY_HOMEPAGE,      VK_BROWSER_HOME,      0,   0),
            NATIVE(KEY_VOLUMEDOWN,    VK_VOLUME_DOWN,       0,   0),
            NATIVE(KEY_VOLUMEMUTE,    VK_VOLUME_MUTE,       0,   0),
            NATIVE(KEY_VOLUMEUP,      VK_VOLUME_UP,         0,   0),
            NATIVE(KEY_MEDIAPLAY,     VK_MEDIA_PLAY_PAUSE,  0,   0),
            NATIVE(KEY_MEDIASTOP,     VK_MEDIA_STOP,        0,   0),
            NATIVE(KEY_MEDIAPREVIOUS, VK_MEDIA_PREV_TRACK,  0,   0),
            NATIVE(KEY_MEDIANEXT,     VK_MEDIA_NEXT_TRACK,  0,   0)
        };
        for (const auto& native : natives) {
            if (native.key == key) return native.code;
        }
        return 0;
    }
    #undef NATIVE

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
