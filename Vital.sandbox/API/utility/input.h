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
            // Alphabet
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

            // Digits
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

            // Numpad Digits
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

            // Numpad Operators
            { "NUM_ADD",      godot::Key::KEY_KP_ADD      },
            { "NUM_SUBTRACT", godot::Key::KEY_KP_SUBTRACT },
            { "NUM_MULTIPLY", godot::Key::KEY_KP_MULTIPLY },
            { "NUM_DIVIDE",   godot::Key::KEY_KP_DIVIDE   },
            { "NUM_PERIOD",   godot::Key::KEY_KP_PERIOD   },

            // Function Keys
            { "F1",  godot::Key::KEY_F1  },
            { "F2",  godot::Key::KEY_F2  },
            { "F3",  godot::Key::KEY_F3  },
            { "F4",  godot::Key::KEY_F4  },
            { "F5",  godot::Key::KEY_F5  },
            { "F6",  godot::Key::KEY_F6  },
            { "F7",  godot::Key::KEY_F7  },
            { "F8",  godot::Key::KEY_F8  },
            { "F9",  godot::Key::KEY_F9  },
            { "F10", godot::Key::KEY_F10 },
            { "F11", godot::Key::KEY_F11 },
            { "F12", godot::Key::KEY_F12 },
            { "F13", godot::Key::KEY_F13 },
            { "F14", godot::Key::KEY_F14 },
            { "F15", godot::Key::KEY_F15 },
            { "F16", godot::Key::KEY_F16 },
            { "F17", godot::Key::KEY_F17 },
            { "F18", godot::Key::KEY_F18 },
            { "F19", godot::Key::KEY_F19 },
            { "F20", godot::Key::KEY_F20 },
            { "F21", godot::Key::KEY_F21 },
            { "F22", godot::Key::KEY_F22 },
            { "F23", godot::Key::KEY_F23 },
            { "F24", godot::Key::KEY_F24 },

            // Arrow Keys
            { "UP",    godot::Key::KEY_UP    },
            { "DOWN",  godot::Key::KEY_DOWN  },
            { "LEFT",  godot::Key::KEY_LEFT  },
            { "RIGHT", godot::Key::KEY_RIGHT },

            // Control Keys
            { "SPACE",     godot::Key::KEY_SPACE     },
            { "ENTER",     godot::Key::KEY_ENTER     },
            { "KP_ENTER",  godot::Key::KEY_KP_ENTER  },
            { "ESCAPE",    godot::Key::KEY_ESCAPE    },
            { "TAB",       godot::Key::KEY_TAB       },
            { "BACKTAB",   godot::Key::KEY_BACKTAB   },
            { "BACKSPACE", godot::Key::KEY_BACKSPACE },
            { "INSERT",    godot::Key::KEY_INSERT    },
            { "DELETE",    godot::Key::KEY_DELETE    },
            { "HOME",      godot::Key::KEY_HOME      },
            { "END",       godot::Key::KEY_END       },
            { "PAGEUP",    godot::Key::KEY_PAGEUP    },
            { "PAGEDOWN",  godot::Key::KEY_PAGEDOWN  },
            { "PAUSE",     godot::Key::KEY_PAUSE     },
            { "PRINT",     godot::Key::KEY_PRINT     },
            { "CLEAR",     godot::Key::KEY_CLEAR     },
            { "MENU",      godot::Key::KEY_MENU      },

            // Modifier Keys (with sided aliases)
            { "SHIFT",  godot::Key::KEY_SHIFT },
            { "LSHIFT", godot::Key::KEY_SHIFT },
            { "RSHIFT", godot::Key::KEY_SHIFT },
            { "CTRL",   godot::Key::KEY_CTRL  },
            { "LCTRL",  godot::Key::KEY_CTRL  },
            { "RCTRL",  godot::Key::KEY_CTRL  },
            { "ALT",    godot::Key::KEY_ALT   },
            { "LALT",   godot::Key::KEY_ALT   },
            { "RALT",   godot::Key::KEY_ALT   },
            { "META",   godot::Key::KEY_META  },
            { "LMETA",  godot::Key::KEY_META  },
            { "RMETA",  godot::Key::KEY_META  },

            // Lock Keys
            { "CAPSLOCK",   godot::Key::KEY_CAPSLOCK   },
            { "NUMLOCK",    godot::Key::KEY_NUMLOCK    },
            { "SCROLLLOCK", godot::Key::KEY_SCROLLLOCK },

            // Punctuation & Symbols
            { "SPACE",        godot::Key::KEY_SPACE        },
            { "EXCLAM",       godot::Key::KEY_EXCLAM       },
            { "QUOTEDBL",     godot::Key::KEY_QUOTEDBL     },
            { "NUMBERSIGN",   godot::Key::KEY_NUMBERSIGN   },
            { "DOLLAR",       godot::Key::KEY_DOLLAR       },
            { "PERCENT",      godot::Key::KEY_PERCENT      },
            { "AMPERSAND",    godot::Key::KEY_AMPERSAND    },
            { "APOSTROPHE",   godot::Key::KEY_APOSTROPHE   },
            { "PARENLEFT",    godot::Key::KEY_PARENLEFT    },
            { "PARENRIGHT",   godot::Key::KEY_PARENRIGHT   },
            { "ASTERISK",     godot::Key::KEY_ASTERISK     },
            { "PLUS",         godot::Key::KEY_PLUS         },
            { "COMMA",        godot::Key::KEY_COMMA        },
            { "MINUS",        godot::Key::KEY_MINUS        },
            { "PERIOD",       godot::Key::KEY_PERIOD       },
            { "SLASH",        godot::Key::KEY_SLASH        },
            { "COLON",        godot::Key::KEY_COLON        },
            { "SEMICOLON",    godot::Key::KEY_SEMICOLON    },
            { "LESS",         godot::Key::KEY_LESS         },
            { "EQUAL",        godot::Key::KEY_EQUAL        },
            { "GREATER",      godot::Key::KEY_GREATER      },
            { "QUESTION",     godot::Key::KEY_QUESTION     },
            { "AT",           godot::Key::KEY_AT           },
            { "BRACKETLEFT",  godot::Key::KEY_BRACKETLEFT  },
            { "BACKSLASH",    godot::Key::KEY_BACKSLASH    },
            { "BRACKETRIGHT", godot::Key::KEY_BRACKETRIGHT },
            { "ASCIICIRCUM",  godot::Key::KEY_ASCIICIRCUM  },
            { "UNDERSCORE",   godot::Key::KEY_UNDERSCORE   },
            { "QUOTELEFT",    godot::Key::KEY_QUOTELEFT    },
            { "BRACELEFT",    godot::Key::KEY_BRACELEFT    },
            { "BAR",          godot::Key::KEY_BAR          },
            { "BRACERIGHT",   godot::Key::KEY_BRACERIGHT   },
            { "ASCIITILDE",   godot::Key::KEY_ASCIITILDE   },

            // Volume & Media Keys
            { "VOLUMEDOWN",    godot::Key::KEY_VOLUMEDOWN    },
            { "VOLUMEMUTE",    godot::Key::KEY_VOLUMEMUTE    },
            { "VOLUMEUP",      godot::Key::KEY_VOLUMEUP      },
            { "MEDIAPLAY",     godot::Key::KEY_MEDIAPLAY     },
            { "MEDIASTOP",     godot::Key::KEY_MEDIASTOP     },
            { "MEDIAPREVIOUS", godot::Key::KEY_MEDIAPREVIOUS },
            { "MEDIANEXT",     godot::Key::KEY_MEDIANEXT     },
            { "MEDIARECORD",   godot::Key::KEY_MEDIARECORD   },

            // Mouse Keys
            { "MOUSE_LEFT",       godot::MouseButton::MOUSE_BUTTON_LEFT       },
            { "MOUSE_RIGHT",      godot::MouseButton::MOUSE_BUTTON_RIGHT      },
            { "MOUSE_MIDDLE",     godot::MouseButton::MOUSE_BUTTON_MIDDLE     },
            { "MOUSE_WHEEL_UP",   godot::MouseButton::MOUSE_BUTTON_WHEEL_UP   },
            { "MOUSE_WHEEL_DOWN", godot::MouseButton::MOUSE_BUTTON_WHEEL_DOWN },
            { "MOUSE_XBUTTON1",   godot::MouseButton::MOUSE_BUTTON_XBUTTON1   },
            { "MOUSE_XBUTTON2",   godot::MouseButton::MOUSE_BUTTON_XBUTTON2   }
        };

        inline static const std::vector<std::pair<std::string, int>> mouse_mode_registry = {
            { "VISIBLE",         godot::Input::MOUSE_MODE_VISIBLE         },
            { "HIDDEN",          godot::Input::MOUSE_MODE_HIDDEN          },
            { "CAPTURED",        godot::Input::MOUSE_MODE_CAPTURED        },
            { "CONFINED",        godot::Input::MOUSE_MODE_CONFINED        },
            { "CONFINED_HIDDEN", godot::Input::MOUSE_MODE_CONFINED_HIDDEN }
        };

        struct Handler {
            int exec_ref = LUA_NOREF;
            bool down;
        };

        inline static std::unordered_map<int, std::unordered_map<std::string, std::vector<Handler>>> bound_keys;
        inline static std::unordered_map<int, std::unordered_map<std::string, std::vector<Handler>>> bound_mouse;

        static bool resolve_key(const std::string& key, int& code, bool& mouse) {
            auto it = std::find_if(key_registry.begin(), key_registry.end(), [&](const auto& p) { return p.first == key; });
            if (it == key_registry.end()) return false;
            code = it -> second;
            mouse = it -> first.rfind("MOUSE_", 0) == 0;
            return true;
        }

        static bool resolve_direction(const std::string& direction, bool& down) {
            if (direction == "down") {down = true; return true; }
            if (direction == "up") { down = false; return true; }
            return false;
        }

        static bool is_valid_key(const std::string& key) {
            int code;
            bool mouse;
            return resolve_key(key, code, mouse);
        }

        static bool is_valid_direction(const std::string& direction) {
            bool down;
            return resolve_direction(direction, down);
        }

        static bool bind_handler(std::unordered_map<int, std::unordered_map<std::string, std::vector<Handler>>>& map, Machine* vm, int code, bool down, int exec_index) {
            auto env = vm -> get_environment_id();
            int ref = vm -> set_raw_reference(exec_index);
            map[code][env].push_back({ref, down});
            return true;
        }

        static bool unbind_handler(std::unordered_map<int, std::unordered_map<std::string, std::vector<Handler>>>& map, Machine* vm, int code, bool down, int exec_index) {
            auto env = vm -> get_environment_id();
            auto mit = map.find(code);
            if (mit == map.end()) return false;
            auto eit = mit -> second.find(env);
            if (eit == mit -> second.end()) return false;

            int lookup_ref = vm -> set_raw_reference(exec_index);
            bool removed = false;
            auto& handlers = eit -> second;
            for (auto vit = handlers.begin(); vit != handlers.end(); ++vit) {
                if (vit -> down != down) continue;
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

        static void dispatch_handler(const std::unordered_map<int, std::unordered_map<std::string, std::vector<Handler>>>& map, Machine* vm, int code, bool pressed, const std::string& key) {
            auto it = map.find(code);
            if (it == map.end()) return;
            auto snapshot = it -> second;
            const std::string direction = pressed ? "down" : "up";
            for (auto& [env, handlers] : snapshot) {
                for (auto& entry : handlers) {
                    if (entry.down != pressed) continue;
                    vm -> get_raw_reference(entry.exec_ref);
                    vm -> push_value(key);
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
                auto pressed = args.array[1].as<bool>();
                auto mouse = args.array[2].as<bool>();
                std::string key_idx;
                for (auto& [key, value] : Input::key_registry) {
                    if (value == code) {
                        key_idx = key;
                        break;
                    }
                }
                if (mouse) dispatch_handler(bound_mouse, vm, code, pressed, key_idx);
                else dispatch_handler(bound_keys, vm, code, pressed, key_idx);
            });
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "is_pressed", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(key)")
                    .require(1, &Machine::is_string)
                    .validate(1, [](Machine* vm, int idx) { return is_valid_key(vm -> get_string(idx)); }, "invalid key");

                int code;
                bool mouse;
                resolve_key(vm -> get_string(1), code, mouse);
                if (mouse) vm -> push_value(godot::Input::get_singleton() -> is_mouse_button_pressed(static_cast<godot::MouseButton>(code)));
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
                vm_args(vm, id, "(key, direction, exec)")
                    .require(1, &Machine::is_string)
                    .validate(1, [](Machine* vm, int idx) { return is_valid_key(vm -> get_string(idx)); }, "invalid key")
                    .require(2, &Machine::is_string)
                    .validate(2, [](Machine* vm, int idx) { return is_valid_direction(vm -> get_string(idx)); }, "direction must be either 'up' or 'down'")
                    .require(3, &Machine::is_function);

                int code;
                bool mouse;
                resolve_key(vm -> get_string(1), code, mouse);
                bool down;
                resolve_direction(vm -> get_string(2), down);
                bool ok = mouse ? bind_handler(bound_mouse, vm, code, down, 3) : bind_handler(bound_keys, vm, code, down, 3);
                vm -> push_value(ok);
                return 1;
            });

            API::bind(vm, base_scope, "unbind", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(key, direction, exec)")
                    .require(1, &Machine::is_string)
                    .validate(1, [](Machine* vm, int idx) { return is_valid_key(vm -> get_string(idx)); }, "invalid key")
                    .require(2, &Machine::is_string)
                    .validate(2, [](Machine* vm, int idx) { return is_valid_direction(vm -> get_string(idx)); }, "direction must be either 'up' or 'down'")
                    .require(3, &Machine::is_function);

                int code;
                bool mouse;
                resolve_key(vm -> get_string(1), code, mouse);
                bool down;
                resolve_direction(vm -> get_string(2), down);
                bool ok = mouse ? unbind_handler(bound_mouse, vm, code, down, 3) : unbind_handler(bound_keys, vm, code, down, 3);
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

            auto release = [&](std::unordered_map<int, std::unordered_map<std::string, std::vector<Handler>>>& map) {
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
            release(bound_keys);
            release(bound_mouse);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Input : vm_module {};
}
#endif
