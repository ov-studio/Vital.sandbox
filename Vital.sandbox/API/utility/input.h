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
            { "NUM9", godot::Key::KEY_KP_9 },
            { "MOUSE_LEFT", godot::MouseButton::MOUSE_BUTTON_LEFT },
            { "MOUSE_RIGHT", godot::MouseButton::MOUSE_BUTTON_RIGHT },
            { "MOUSE_MIDDLE", godot::MouseButton::MOUSE_BUTTON_MIDDLE },
            { "MOUSE_WHEEL_UP", godot::MouseButton::MOUSE_BUTTON_WHEEL_UP },
            { "MOUSE_WHEEL_DOWN", godot::MouseButton::MOUSE_BUTTON_WHEEL_DOWN },
            { "MOUSE_XBUTTON1", godot::MouseButton::MOUSE_BUTTON_XBUTTON1 },
            { "MOUSE_XBUTTON2", godot::MouseButton::MOUSE_BUTTON_XBUTTON2 }
        };

        inline static const std::vector<std::pair<std::string, int>> mouse_mode_registry = {
            { "VISIBLE", godot::Input::MOUSE_MODE_VISIBLE },
            { "HIDDEN", godot::Input::MOUSE_MODE_HIDDEN },
            { "CAPTURED", godot::Input::MOUSE_MODE_CAPTURED },
            { "CONFINED", godot::Input::MOUSE_MODE_CONFINED },
            { "CONFINED_HIDDEN", godot::Input::MOUSE_MODE_CONFINED_HIDDEN }
        };

        struct Handler {
            int exec_ref = LUA_NOREF;
            bool is_down;
        };

        inline static std::unordered_map<int, std::unordered_map<std::string, std::vector<Handler>>> bound_keys;
        inline static std::unordered_map<int, std::unordered_map<std::string, std::vector<Handler>>> bound_mouse;

        static bool resolve_key(const std::string& name, bool& is_mouse, int& code) {
            auto it = std::find_if(key_registry.begin(), key_registry.end(), [&](const auto& p) { return p.first == name; });
            if (it == key_registry.end()) return false;
            is_mouse = it -> first.rfind("MOUSE_", 0) == 0;
            code = it -> second;
            return true;
        }

        static bool is_valid_key(const std::string& name) {
            bool is_mouse; int code;
            return resolve_key(name, is_mouse, code);
        }

        static bool bind_handler(std::unordered_map<int, std::unordered_map<std::string, std::vector<Handler>>>& map, Machine* vm, int code, bool is_down, int exec_index) {
            auto env = vm -> get_environment_id();
            int ref = vm -> set_raw_reference(exec_index);
            map[code][env].push_back({ref, is_down});
            return true;
        }

        static bool unbind_handler(std::unordered_map<int, std::unordered_map<std::string, std::vector<Handler>>>& map, Machine* vm, int code, bool is_down, int exec_index) {
            auto env = vm -> get_environment_id();
            auto mit = map.find(code);
            if (mit == map.end()) return false;
            auto eit = mit -> second.find(env);
            if (eit == mit -> second.end()) return false;

            int lookup_ref = vm -> set_raw_reference(exec_index);
            bool removed = false;
            auto& handlers = eit -> second;
            for (auto vit = handlers.begin(); vit != handlers.end(); ++vit) {
                if (vit -> is_down != is_down) continue;
                vm -> get_raw_reference(lookup_ref);
                vm -> get_raw_reference(vit -> exec_ref);
                bool eq = lua_rawequal(vm -> get_state(), -1, -2);
                vm -> pop(2);
                if (!eq) continue;
                vm -> del_raw_reference(vit -> exec_ref);
                handlers.erase(vit);
                removed = true;
                break;
            }
            vm -> del_raw_reference(lookup_ref);

            if (handlers.empty()) mit -> second.erase(eit);
            if (mit -> second.empty()) map.erase(mit);
            return removed;
        }

        static void dispatch_handler(const std::unordered_map<int, std::unordered_map<std::string, std::vector<Handler>>>& map, Machine* vm, int code, bool is_pressed, const std::string& key_name) {
            auto it = map.find(code);
            if (it == map.end()) return;
            auto snapshot = it -> second;
            const std::string direction = is_pressed ? "down" : "up";
            for (auto& [env, handlers] : snapshot) {
                for (auto& entry : handlers) {
                    if (entry.is_down != is_pressed) continue;
                    vm -> get_raw_reference(entry.exec_ref);
                    vm -> push_value(key_name);
                    vm -> push_value(direction);
                    vm -> call(2, 0);
                }
            }
        }

        static void init(Machine* vm) {
            static bool initialized = false;
            if (initialized) return;
            initialized = true;

            Tool::Event::bind("sandbox:key_input", [](Tool::Stack args) {
                if (args.array.size() < 3) return;
                auto vm = Manager::Sandbox::get_singleton() -> get_vm();
                if (!vm) return;

                auto code = args.array[0].as<int32_t>();
                auto is_pressed = args.array[1].as<bool>();
                auto is_mouse = args.array[2].as<bool>();
                std::string key_name;
                for (auto& [name, val] : Input::key_registry) {
                    if (val == code) {
                        key_name = name;
                        break;
                    }
                }
                if (is_mouse) dispatch_handler(bound_mouse, vm, code, is_pressed, key_name);
                else dispatch_handler(bound_keys, vm, code, is_pressed, key_name);
            });
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "is_pressed", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(key)")
                    .require(1, &Machine::is_string)
                    .validate(1, [](Machine* vm, int idx) { return is_valid_key(vm -> get_string(idx)); }, "unknown key binding");

                bool is_mouse; int code;
                resolve_key(vm -> get_string(1), is_mouse, code);
                if (is_mouse) vm -> push_value(godot::Input::get_singleton() -> is_mouse_button_pressed(static_cast<godot::MouseButton>(code)));
                else vm -> push_value(godot::Input::get_singleton() -> is_key_pressed(static_cast<godot::Key>(code)));
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

            API::bind(vm, base_scope, "set_mouse_mode", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(mode)")
                    .require_enum(1, mouse_mode_registry);

                auto mode = static_cast<godot::Input::MouseMode>(vm -> get_int(1));
                godot::Input::get_singleton() -> set_mouse_mode(mode);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "bind", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, direction, exec)")
                    .require(1, &Machine::is_string)
                    .validate(1, [](Machine* vm, int idx) { return is_valid_key(vm -> get_string(idx)); }, "unknown key binding")
                    .require(2, &Machine::is_string)
                    .validate(2, [](Machine* vm, int idx) {
                        auto dir = vm -> get_string(idx);
                        return dir == "up" || dir == "down";
                    }, "direction must be 'up' or 'down'")
                    .require(3, &Machine::is_function);

                bool is_mouse; int code;
                resolve_key(vm -> get_string(1), is_mouse, code);
                bool is_down = vm -> get_string(2) == "down";
                bool ok = is_mouse ? bind_handler(bound_mouse, vm, code, is_down, 3) : bind_handler(bound_keys, vm, code, is_down, 3);
                vm -> push_value(ok);
                return 1;
            });

            API::bind(vm, base_scope, "unbind", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, direction, exec)")
                    .require(1, &Machine::is_string)
                    .validate(1, [](Machine* vm, int idx) { return is_valid_key(vm -> get_string(idx)); }, "unknown key binding")
                    .require(2, &Machine::is_string)
                    .validate(2, [](Machine* vm, int idx) {
                        auto dir = vm -> get_string(idx);
                        return dir == "up" || dir == "down";
                    }, "direction must be 'up' or 'down'")
                    .require(3, &Machine::is_function);

                bool is_mouse; int code;
                resolve_key(vm -> get_string(1), is_mouse, code);
                bool is_down = vm -> get_string(2) == "down";
                bool ok = is_mouse ? unbind_handler(bound_mouse, vm, code, is_down, 3) : unbind_handler(bound_keys, vm, code, is_down, 3);
                vm -> push_value(ok);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "key", key_registry);
            vm -> scope_set_enum(base_scope, "mouse_mode", mouse_mode_registry);
        }

        static void clean(const std::string& env) {
            auto vm = Manager::Sandbox::get_singleton() -> get_vm();
            if (!vm) return;

            auto release_bound = [&](std::unordered_map<int, std::unordered_map<std::string, std::vector<Handler>>>& map) {
                for (auto mit = map.begin(); mit != map.end(); ) {
                    auto eit = mit -> second.find(env);
                    if (eit != mit -> second.end()) {
                        for (auto& entry : eit -> second) vm -> del_raw_reference(entry.exec_ref);
                        mit -> second.erase(eit);
                    }
                    if (mit -> second.empty()) mit = map.erase(mit);
                    else ++mit;
                }
            };
            release_bound(bound_keys);
            release_bound(bound_mouse);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Input : vm_module {};
}
#endif