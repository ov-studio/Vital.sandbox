/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: input.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Input APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>


////////////////////////
// Vital: API: Input //
////////////////////////

namespace Vital::Sandbox::API {
    struct Input : vm_module {
        inline static const std::vector<std::string> base_scope = {"util", "input"};
        inline static const std::string mouse_prefix = "mouse_";

        inline static const std::vector<std::pair<std::string, int>> key_registry = {
            { "A", godot::Key::KEY_A },
            { "B", godot::Key::KEY_B },
            { "C", godot::Key::KEY_C },
            { "D", godot::Key::KEY_D },
            { "E", godot::Key::KEY_E },
            { "F", godot::Key::KEY_F },
            { "G", godot::Key::KEY_G },
            { "H", godot::Key::KEY_H },
            { "I", godot::Key::KEY_I },
            { "J", godot::Key::KEY_J },
            { "K", godot::Key::KEY_K },
            { "L", godot::Key::KEY_L },
            { "M", godot::Key::KEY_M },
            { "N", godot::Key::KEY_N },
            { "O", godot::Key::KEY_O },
            { "P", godot::Key::KEY_P },
            { "Q", godot::Key::KEY_Q },
            { "R", godot::Key::KEY_R },
            { "S", godot::Key::KEY_S },
            { "T", godot::Key::KEY_T },
            { "U", godot::Key::KEY_U },
            { "V", godot::Key::KEY_V },
            { "W", godot::Key::KEY_W },
            { "X", godot::Key::KEY_X },
            { "Y", godot::Key::KEY_Y },
            { "Z", godot::Key::KEY_Z },
            { "0", godot::Key::KEY_0 },
            { "1", godot::Key::KEY_1 },
            { "2", godot::Key::KEY_2 },
            { "3", godot::Key::KEY_3 },
            { "4", godot::Key::KEY_4 },
            { "5", godot::Key::KEY_5 },
            { "6", godot::Key::KEY_6 },
            { "7", godot::Key::KEY_7 },
            { "8", godot::Key::KEY_8 },
            { "9", godot::Key::KEY_9 },
            { "F1", godot::Key::KEY_F1 },
            { "F2", godot::Key::KEY_F2 },
            { "F3", godot::Key::KEY_F3 },
            { "F4", godot::Key::KEY_F4 },
            { "F5", godot::Key::KEY_F5 },
            { "F6", godot::Key::KEY_F6 },
            { "F7", godot::Key::KEY_F7 },
            { "F8", godot::Key::KEY_F8 },
            { "F9", godot::Key::KEY_F9 },
            { "F10", godot::Key::KEY_F10 },
            { "F11", godot::Key::KEY_F11 },
            { "F12", godot::Key::KEY_F12 },
            { "UP", godot::Key::KEY_UP },
            { "DOWN", godot::Key::KEY_DOWN },
            { "LEFT", godot::Key::KEY_LEFT },
            { "RIGHT", godot::Key::KEY_RIGHT },
            { "SPACE", godot::Key::KEY_SPACE },
            { "ENTER", godot::Key::KEY_ENTER },
            { "ESCAPE", godot::Key::KEY_ESCAPE },
            { "TAB", godot::Key::KEY_TAB },
            { "BACKSPACE", godot::Key::KEY_BACKSPACE },
            { "SHIFT", godot::Key::KEY_SHIFT },
            { "LSHIFT", godot::Key::KEY_SHIFT },
            { "RSHIFT", godot::Key::KEY_SHIFT },
            { "CTRL", godot::Key::KEY_CTRL },
            { "LCTRL", godot::Key::KEY_CTRL },
            { "RCTRL", godot::Key::KEY_CTRL },
            { "ALT", godot::Key::KEY_ALT },
            { "LALT", godot::Key::KEY_ALT },
            { "RALT", godot::Key::KEY_ALT },
            { "META", godot::Key::KEY_META },
            { "LMETA", godot::Key::KEY_META },
            { "RMETA", godot::Key::KEY_META },
            { "CAPSLOCK", godot::Key::KEY_CAPSLOCK },
            { "INSERT", godot::Key::KEY_INSERT },
            { "DELETE", godot::Key::KEY_DELETE },
            { "HOME", godot::Key::KEY_HOME },
            { "END", godot::Key::KEY_END },
            { "PAGEUP", godot::Key::KEY_PAGEUP },
            { "PAGEDOWN", godot::Key::KEY_PAGEDOWN },
            { "NUM0", godot::Key::KEY_KP_0 },
            { "NUM1", godot::Key::KEY_KP_1 },
            { "NUM2", godot::Key::KEY_KP_2 },
            { "NUM3", godot::Key::KEY_KP_3 },
            { "NUM4", godot::Key::KEY_KP_4 },
            { "NUM5", godot::Key::KEY_KP_5 },
            { "NUM6", godot::Key::KEY_KP_6 },
            { "NUM7", godot::Key::KEY_KP_7 },
            { "NUM8", godot::Key::KEY_KP_8 },
            { "NUM9", godot::Key::KEY_KP_9 }
        };

        inline static const std::vector<std::pair<std::string, int>> mouse_button_registry = {
            { "LEFT", godot::MouseButton::MOUSE_BUTTON_LEFT },
            { "RIGHT", godot::MouseButton::MOUSE_BUTTON_RIGHT },
            { "MIDDLE", godot::MouseButton::MOUSE_BUTTON_MIDDLE },
            { "WHEEL_UP", godot::MouseButton::MOUSE_BUTTON_WHEEL_UP },
            { "WHEEL_DOWN", godot::MouseButton::MOUSE_BUTTON_WHEEL_DOWN },
            { "XBUTTON1", godot::MouseButton::MOUSE_BUTTON_XBUTTON1 },
            { "XBUTTON2", godot::MouseButton::MOUSE_BUTTON_XBUTTON2 }
        };

        inline static const std::vector<std::pair<std::string, int>> mouse_mode_registry = {
            { "VISIBLE", godot::Input::MOUSE_MODE_VISIBLE },
            { "HIDDEN", godot::Input::MOUSE_MODE_HIDDEN },
            { "CAPTURED", godot::Input::MOUSE_MODE_CAPTURED },
            { "CONFINED", godot::Input::MOUSE_MODE_CONFINED },
            { "CONFINED_HIDDEN", godot::Input::MOUSE_MODE_CONFINED_HIDDEN }
        };

        // Per-key bound handlers across all envs: keycode -> { env -> { ref_down, ref_up } }
        inline static std::unordered_map<int, std::unordered_map<std::string, std::pair<int, int>>> bound_keys;
        // Per-mouse-button bound handlers: button -> { env -> { ref_down, ref_up } }
        inline static std::unordered_map<int, std::unordered_map<std::string, std::pair<int, int>>> bound_mouse;


        // Helpers //
        // Shared by bind_key/bind_mouse — registers a fresh on_down/on_up pair
        // for the calling env, releasing any prior binding it held first.
        static void bind_pair(std::unordered_map<int, std::unordered_map<std::string, std::pair<int, int>>>& map, Machine* vm, int code, int down_index, int up_index) {
            auto env = vm -> get_environment_id();
            auto& env_map = map[code];
            auto it = env_map.find(env);
            if (it != env_map.end()) {
                vm -> del_raw_reference(it -> second.first);
                vm -> del_raw_reference(it -> second.second);
                env_map.erase(it);
            }
            int ref_up   = vm -> set_raw_reference(up_index);
            int ref_down = vm -> set_raw_reference(down_index);
            env_map[env] = { ref_down, ref_up };
        }

        // Shared by unbind_key/unbind_mouse — releases the calling env's
        // binding for a code, if any, and prunes now-empty map entries.
        static void unbind_pair(std::unordered_map<int, std::unordered_map<std::string, std::pair<int, int>>>& map, Machine* vm, int code) {
            auto env = vm -> get_environment_id();
            auto mit = map.find(code);
            if (mit == map.end()) return;
            auto eit = mit -> second.find(env);
            if (eit != mit -> second.end()) {
                vm -> del_raw_reference(eit -> second.first);
                vm -> del_raw_reference(eit -> second.second);
                mit -> second.erase(eit);
            }
            if (mit -> second.empty()) map.erase(mit);
        }

        // Shared by the sandbox:key_input handler — fires every env's
        // on_down/on_up callback bound to a given code, if any.
        static void dispatch_pair(const std::unordered_map<int, std::unordered_map<std::string, std::pair<int, int>>>& map, Machine* vm, int code, bool is_pressed) {
            auto it = map.find(code);
            if (it == map.end()) return;
            for (auto& [env, refs] : it -> second) {
                int ref = is_pressed ? refs.first : refs.second;
                if (ref == LUA_NOREF) continue;
                vm -> get_raw_reference(ref);
                vm -> call(0, 0);
            }
        }

        static void init(Machine* vm) {
            static bool initialized = false;
            if (initialized) return;
            initialized = true;

            // Hook into existing sandbox:key_input signal (fired by Manager::Sandbox::input())
            Tool::Event::bind("sandbox:key_input", [vm](Tool::Stack args) {
                if (args.array.size() < 2) return;
                auto key_str = args.array[0].as<std::string>();
                bool is_pressed = args.array[1].as<bool>();

                if (key_str.rfind(mouse_prefix, 0) == 0) dispatch_pair(bound_mouse, vm, std::stoi(key_str.substr(mouse_prefix.size())), is_pressed);
                else dispatch_pair(bound_keys, vm, std::stoi(key_str), is_pressed);
            });
        }

        static void bind(Machine* vm) {
            // Checkers //
            API::bind(vm, base_scope, "is_key_pressed", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(key)")
                    .require_enum(1, key_registry);

                auto key = static_cast<godot::Key>(vm -> get_int(1));
                vm -> push_value(godot::Input::get_singleton() -> is_key_pressed(key));
                return 1;
            });

            API::bind(vm, base_scope, "is_mouse_pressed", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(button)")
                    .require_enum(1, mouse_button_registry);

                auto button = static_cast<godot::MouseButton>(vm -> get_int(1));
                vm -> push_value(godot::Input::get_singleton() -> is_mouse_button_pressed(button));
                return 1;
            });


            // Getters //
            API::bind(vm, base_scope, "get_key_state", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(key)")
                    .require_enum(1, key_registry);

                auto key = static_cast<godot::Key>(vm -> get_int(1));
                if (godot::Input::get_singleton() -> is_key_pressed(key)) vm -> push_value(std::string("down"));
                else vm -> push_value(std::string("up"));
                return 1;
            });

            API::bind(vm, base_scope, "get_mouse_velocity", [](auto vm, auto& id) -> int {
                vm -> push_value(godot::Input::get_singleton() -> get_last_mouse_velocity());
                return 1;
            });

            API::bind(vm, base_scope, "get_mouse_mode", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(godot::Input::get_singleton() -> get_mouse_mode()));
                return 1;
            });

            API::bind(vm, base_scope, "get_action_strength", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(action)")
                    .require(1, &Machine::is_string);

                auto action = Tool::to_godot_string_name(vm -> get_string(1));
                vm -> push_value(godot::Input::get_singleton() -> get_action_strength(action));
                return 1;
            });

            API::bind(vm, base_scope, "get_axis", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(negative_action, positive_action)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_string);

                auto neg = Tool::to_godot_string_name(vm -> get_string(1));
                auto pos = Tool::to_godot_string_name(vm -> get_string(2));
                vm -> push_value(godot::Input::get_singleton() -> get_axis(neg, pos));
                return 1;
            });

            // Setters //
            API::bind(vm, base_scope, "set_mouse_mode", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(mode)")
                    .require_enum(1, mouse_mode_registry);

                auto mode = static_cast<godot::Input::MouseMode>(vm -> get_int(1));
                godot::Input::get_singleton() -> set_mouse_mode(mode);
                vm -> push_value(true);
                return 1;
            });

            // Bind Key //
            API::bind(vm, base_scope, "bind_key", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(key, on_down, on_up)")
                    .require_enum(1, key_registry)
                    .require(2, &Machine::is_function)
                    .require(3, &Machine::is_function);

                bind_pair(bound_keys, vm, vm -> get_int(1), 2, 3);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "unbind_key", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(key)")
                    .require_enum(1, key_registry);

                unbind_pair(bound_keys, vm, vm -> get_int(1));
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "bind_mouse", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(button, on_down, on_up)")
                    .require_enum(1, mouse_button_registry)
                    .require(2, &Machine::is_function)
                    .require(3, &Machine::is_function);

                bind_pair(bound_mouse, vm, vm -> get_int(1), 2, 3);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "unbind_mouse", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(button)")
                    .require_enum(1, mouse_button_registry);

                unbind_pair(bound_mouse, vm, vm -> get_int(1));
                vm -> push_value(true);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "key", key_registry);
            vm -> scope_set_enum(base_scope, "mouse_button", mouse_button_registry);
            vm -> scope_set_enum(base_scope, "mouse_mode", mouse_mode_registry);
        }

        static void clean(const std::string& env) {
            // Free all key binds belonging to this env
            for (auto kit = bound_keys.begin(); kit != bound_keys.end(); ) {
                auto eit = kit -> second.find(env);
                if (eit != kit -> second.end()) {
                    // refs already freed by Lua GC on env clear; just erase tracking
                    kit -> second.erase(eit);
                }
                if (kit -> second.empty()) kit = bound_keys.erase(kit);
                else ++kit;
            }
            for (auto bit = bound_mouse.begin(); bit != bound_mouse.end(); ) {
                auto eit = bit -> second.find(env);
                if (eit != bit -> second.end()) {
                    bit -> second.erase(eit);
                }
                if (bit -> second.empty()) bit = bound_mouse.erase(bit);
                else ++bit;
            }
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Input : vm_module {};
}
#endif