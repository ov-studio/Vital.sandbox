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
            { "NUM_0", godot::Key::KEY_KP_0 },
            { "NUM_1", godot::Key::KEY_KP_1 },
            { "NUM_2", godot::Key::KEY_KP_2 },
            { "NUM_3", godot::Key::KEY_KP_3 },
            { "NUM_4", godot::Key::KEY_KP_4 },
            { "NUM_5", godot::Key::KEY_KP_5 },
            { "NUM_6", godot::Key::KEY_KP_6 },
            { "NUM_7", godot::Key::KEY_KP_7 },
            { "NUM_8", godot::Key::KEY_KP_8 },
            { "NUM_9", godot::Key::KEY_KP_9 },

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
            { "F25", godot::Key::KEY_F25 },
            { "F26", godot::Key::KEY_F26 },
            { "F27", godot::Key::KEY_F27 },
            { "F28", godot::Key::KEY_F28 },
            { "F29", godot::Key::KEY_F29 },
            { "F30", godot::Key::KEY_F30 },
            { "F31", godot::Key::KEY_F31 },
            { "F32", godot::Key::KEY_F32 },
            { "F33", godot::Key::KEY_F33 },
            { "F34", godot::Key::KEY_F34 },
            { "F35", godot::Key::KEY_F35 },

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
            { "PAGE_UP",   godot::Key::KEY_PAGEUP    },
            { "PAGE_DOWN", godot::Key::KEY_PAGEDOWN  },
            { "PAUSE",     godot::Key::KEY_PAUSE     },
            { "PRINT",     godot::Key::KEY_PRINT     },
            { "CLEAR",     godot::Key::KEY_CLEAR     },
            { "MENU",      godot::Key::KEY_MENU      },
            { "SYSREQ",    godot::Key::KEY_SYSREQ    },
            { "HYPER",     godot::Key::KEY_HYPER     },
            { "HELP",      godot::Key::KEY_HELP      },
            { "BACK",      godot::Key::KEY_BACK      },
            { "FORWARD",   godot::Key::KEY_FORWARD   },
            { "STOP",      godot::Key::KEY_STOP      },
            { "REFRESH",   godot::Key::KEY_REFRESH   },

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
            { "CAPS_LOCK",   godot::Key::KEY_CAPSLOCK   },
            { "NUM_LOCK",    godot::Key::KEY_NUMLOCK    },
            { "SCROLL_LOCK", godot::Key::KEY_SCROLLLOCK },

            // Punctuation & Symbols
            { "EXCLAM",        godot::Key::KEY_EXCLAM       },
            { "QUOTEDBL",      godot::Key::KEY_QUOTEDBL     },
            { "NUMBER_SIGN",   godot::Key::KEY_NUMBERSIGN   },
            { "DOLLAR",        godot::Key::KEY_DOLLAR       },
            { "PERCENT",       godot::Key::KEY_PERCENT      },
            { "AMPERSAND",     godot::Key::KEY_AMPERSAND    },
            { "APOSTROPHE",    godot::Key::KEY_APOSTROPHE   },
            { "PAREN_LEFT",    godot::Key::KEY_PARENLEFT    },
            { "PAREN_RIGHT",   godot::Key::KEY_PARENRIGHT   },
            { "ASTERISK",      godot::Key::KEY_ASTERISK     },
            { "PLUS",          godot::Key::KEY_PLUS         },
            { "COMMA",         godot::Key::KEY_COMMA        },
            { "MINUS",         godot::Key::KEY_MINUS        },
            { "PERIOD",        godot::Key::KEY_PERIOD       },
            { "SLASH",         godot::Key::KEY_SLASH        },
            { "COLON",         godot::Key::KEY_COLON        },
            { "SEMICOLON",     godot::Key::KEY_SEMICOLON    },
            { "LESS",          godot::Key::KEY_LESS         },
            { "EQUAL",         godot::Key::KEY_EQUAL        },
            { "GREATER",       godot::Key::KEY_GREATER      },
            { "QUESTION",      godot::Key::KEY_QUESTION     },
            { "AT",            godot::Key::KEY_AT           },
            { "BRACKET_LEFT",  godot::Key::KEY_BRACKETLEFT  },
            { "BACKSLASH",     godot::Key::KEY_BACKSLASH    },
            { "BRACKET_RIGHT", godot::Key::KEY_BRACKETRIGHT },
            { "CIRCUM",        godot::Key::KEY_ASCIICIRCUM  },
            { "UNDERSCORE",    godot::Key::KEY_UNDERSCORE   },
            { "QUOTE_LEFT",    godot::Key::KEY_QUOTELEFT    },
            { "BRACE_LEFT",    godot::Key::KEY_BRACELEFT    },
            { "BAR",           godot::Key::KEY_BAR          },
            { "BRACE_RIGHT",   godot::Key::KEY_BRACERIGHT   },
            { "TILDE",         godot::Key::KEY_ASCIITILDE   },
            { "YEN",           godot::Key::KEY_YEN          },
            { "SECTION",       godot::Key::KEY_SECTION      },

            // Volume & Media Keys
            { "VOLUME_DOWN",    godot::Key::KEY_VOLUMEDOWN    },
            { "VOLUME_MUTE",    godot::Key::KEY_VOLUMEMUTE    },
            { "VOLUME_UP",      godot::Key::KEY_VOLUMEUP      },
            { "MEDIA_PLAY",     godot::Key::KEY_MEDIAPLAY     },
            { "MEDIA_STOP",     godot::Key::KEY_MEDIASTOP     },
            { "MEDIA_PREVIOUS", godot::Key::KEY_MEDIAPREVIOUS },
            { "MEDIA_NEXT",     godot::Key::KEY_MEDIANEXT     },
            { "MEDIA_RECORD",   godot::Key::KEY_MEDIARECORD   },

            // Browser & Launch Keys
            { "HOMEPAGE",     godot::Key::KEY_HOMEPAGE    },
            { "FAVORITES",    godot::Key::KEY_FAVORITES   },
            { "SEARCH",       godot::Key::KEY_SEARCH      },
            { "STANDBY",      godot::Key::KEY_STANDBY     },
            { "OPEN_URL",     godot::Key::KEY_OPENURL     },
            { "LAUNCH_MAIL",  godot::Key::KEY_LAUNCHMAIL  },
            { "LAUNCH_MEDIA", godot::Key::KEY_LAUNCHMEDIA },
            { "LAUNCH0",      godot::Key::KEY_LAUNCH0     },
            { "LAUNCH1",      godot::Key::KEY_LAUNCH1     },
            { "LAUNCH2",      godot::Key::KEY_LAUNCH2     },
            { "LAUNCH3",      godot::Key::KEY_LAUNCH3     },
            { "LAUNCH4",      godot::Key::KEY_LAUNCH4     },
            { "LAUNCH5",      godot::Key::KEY_LAUNCH5     },
            { "LAUNCH6",      godot::Key::KEY_LAUNCH6     },
            { "LAUNCH7",      godot::Key::KEY_LAUNCH7     },
            { "LAUNCH8",      godot::Key::KEY_LAUNCH8     },
            { "LAUNCH9",      godot::Key::KEY_LAUNCH9     },
            { "LAUNCHA",      godot::Key::KEY_LAUNCHA     },
            { "LAUNCHB",      godot::Key::KEY_LAUNCHB     },
            { "LAUNCHC",      godot::Key::KEY_LAUNCHC     },
            { "LAUNCHD",      godot::Key::KEY_LAUNCHD     },
            { "LAUNCHE",      godot::Key::KEY_LAUNCHE     },
            { "LAUNCHF",      godot::Key::KEY_LAUNCHF     },
            { "GLOBE",        godot::Key::KEY_GLOBE       },
            { "KEYBOARD",     godot::Key::KEY_KEYBOARD    },
            { "JIS_EISU",     godot::Key::KEY_JIS_EISU    },
            { "JIS_KANA",     godot::Key::KEY_JIS_KANA    },

            // Mouse Keys
            { "MOUSE_LEFT",       godot::MouseButton::MOUSE_BUTTON_LEFT       },
            { "MOUSE_RIGHT",      godot::MouseButton::MOUSE_BUTTON_RIGHT      },
            { "MOUSE_MIDDLE",     godot::MouseButton::MOUSE_BUTTON_MIDDLE     },
            { "MOUSE_WHEEL_UP",   godot::MouseButton::MOUSE_BUTTON_WHEEL_UP   },
            { "MOUSE_WHEEL_DOWN", godot::MouseButton::MOUSE_BUTTON_WHEEL_DOWN },
            { "MOUSE_XBUTTON1",   godot::MouseButton::MOUSE_BUTTON_XBUTTON1   },
            { "MOUSE_XBUTTON2",   godot::MouseButton::MOUSE_BUTTON_XBUTTON2   }
        };

        inline static const std::vector<std::pair<std::string, int>> cursor_mode_registry = {
            { "VISIBLE",  godot::Input::MOUSE_MODE_VISIBLE   },
            { "HIDDEN",   godot::Input::MOUSE_MODE_HIDDEN    },
            { "CAPTURED", godot::Input::MOUSE_MODE_CAPTURED  }
        };

        struct BindHandler {
            int exec_ref = LUA_NOREF;
            bool down;
        };

        inline static std::unordered_map<int, std::unordered_map<std::string, std::vector<Handler>>> key_binds;
        inline static std::unordered_map<int, std::unordered_map<std::string, std::vector<Handler>>> mouse_binds;


        static bool resolve_key(int code, std::string& key, bool& mouse) {
            auto it = std::find_if(key_registry.begin(), key_registry.end(), [&](const auto& p) { return p.second == code; });
            if (it == key_registry.end()) return false;
            key = it -> first;
            mouse = it -> first.rfind("MOUSE_", 0) == 0;
            return true;
        }

        static bool resolve_direction(const std::string& direction, bool& down) {
            if (direction == "down") { down = true; return true; }
            if (direction == "up") { down = false; return true; }
            return false;
        }

        static bool is_valid_direction(const std::string& direction) {
            bool down;
            return resolve_direction(direction, down);
        }

        static bool bind_handler(std::unordered_map<int, std::unordered_map<std::string, std::vector<BindHandler>>>& map, Machine* vm, int code, bool down, int exec_index) {
            auto env = vm -> get_environment_id();
            int ref = vm -> set_raw_reference(exec_index);
            map[code][env].push_back({ref, down});
            return true;
        }

        static bool unbind_handler(std::unordered_map<int, std::unordered_map<std::string, std::vector<BindHandler>>>& map, Machine* vm, int code, bool down, int exec_index) {
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

        static void dispatch_handler(const std::unordered_map<int, std::unordered_map<std::string, std::vector<BindHandler>>>& map, Machine* vm, int code, bool pressed) {
            auto it = map.find(code);
            if (it == map.end()) return;
            auto snapshot = it -> second;
            const std::string direction = pressed ? "down" : "up";
            for (auto& [env, handlers] : snapshot) {
                for (auto& entry : handlers) {
                    if (entry.down != pressed) continue;
                    vm -> get_raw_reference(entry.exec_ref);
                    vm -> push_value(code);
                    vm -> push_value(direction);
                    vm -> call(2, 0);
                }
            }
        }

        static bool register_command(Machine* vm, const std::string& name, int exec_index) {
            auto env = vm -> get_environment_id();
            int ref = vm -> set_raw_reference(exec_index);
            command_handlers[name][env].push_back({ref});
            return true;
        }

        static bool unregister_command(Machine* vm, const std::string& name, int exec_index) {
            auto env = vm -> get_environment_id();
            auto cit = command_handlers.find(name);
            if (cit == command_handlers.end()) return false;
            auto eit = cit -> second.find(env);
            if (eit == cit -> second.end()) return false;

            int lookup_ref = vm -> set_raw_reference(exec_index);
            bool removed = false;
            auto& handlers = eit -> second;
            for (auto vit = handlers.begin(); vit != handlers.end(); ++vit) {
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
            if (handlers.empty()) cit -> second.erase(eit);
            if (cit -> second.empty()) command_handlers.erase(cit);
            return removed;
        }

        static std::vector<std::string> list_commands() {
            std::vector<std::string> names;
            names.reserve(command_handlers.size());
            for (auto& [name, _] : command_handlers) names.push_back(name);
            return names;
        }
        
        static bool dispatch_command(Machine* vm, const std::string& name, const std::vector<std::string>& args) {
            auto cit = command_handlers.find(name);
            if (cit == command_handlers.end()) return false;
            auto snapshot = cit -> second;
            for (auto& [env, handlers] : snapshot) {
                for (auto& entry : handlers) {
                    vm -> get_raw_reference(entry.exec_ref);
                    // push args as a Lua table
                    vm -> create_table();
                    for (int i = 0; i < (int)args.size(); ++i) {
                        vm -> push_value(args[i]);
                        vm -> set_table_field(i + 1, -2);
                    }
                    vm -> call(1, 0);
                }
            }
            return true;
        }

        static void init(Machine* vm) {
            static bool initialized = false;
            if (initialized) return;
            initialized = true;

            Tool::Event::bind("sandbox:key_input", [](Tool::Stack args) {
                if (args.array.size() < 2) return;
                auto vm = Manager::Sandbox::get_singleton() -> get_vm();
                if (!vm) return;

                auto code = args.array[0].as<int32_t>();
                auto pressed = args.array[1].as<bool>();
                std::string key;
                bool mouse;
                if (!resolve_key(code, key, mouse)) return;
                if (mouse) dispatch_handler(mouse_binds, vm, code, pressed);
                else dispatch_handler(key_binds, vm, code, pressed);
            });
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "is_pressed", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(key)")
                    .require_enum(1, key_registry);

                auto code = vm -> get_int(1);
                std::string key;
                bool mouse;
                resolve_key(code, key, mouse);
                auto pressed = mouse ? godot::Input::get_singleton() -> is_mouse_button_pressed(static_cast<godot::MouseButton>(code)) : godot::Input::get_singleton() -> is_key_pressed(static_cast<godot::Key>(code));
                vm -> push_value(pressed);
                return 1;
            });

            API::bind(vm, base_scope, "get_cursor_mode", [](auto vm, auto& id) -> int {
                vm -> push_value(static_cast<int>(godot::Input::get_singleton() -> get_mouse_mode()));
                return 1;
            });

            API::bind(vm, base_scope, "get_cursor_position", [](auto vm, auto& id) -> int {
                auto position = Vital::Engine::Core::get_scene_root() -> get_viewport() -> get_mouse_position();
                vm -> push_value(position);
                return 1;
            });
            
            API::bind(vm, base_scope, "get_cursor_velocity", [](auto vm, auto& id) -> int {
                vm -> push_value(godot::Input::get_singleton() -> get_last_mouse_velocity());
                return 1;
            });

            API::bind(vm, base_scope, "set_cursor_mode", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(mode)")
                    .require_enum(1, cursor_mode_registry);

                auto mode = static_cast<godot::Input::MouseMode>(vm -> get_int(1));
                godot::Input::get_singleton() -> set_mouse_mode(mode);
                vm -> push_value(true);
                return 1;
            });
            
            API::bind(vm, base_scope, "set_cursor_position", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(position)")
                    .require(1, &Machine::is_vector2);

                auto position = vm -> get_vector2(1);
                Vital::Engine::Core::get_scene_root() -> get_viewport() -> warp_mouse(position);
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "bind", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(key, direction, exec)")
                    .require_enum(1, key_registry)
                    .require(2, &Machine::is_string)
                    .validate(2, [](Machine* vm, int idx) { return is_valid_direction(vm -> get_string(idx)); }, "direction must be either 'up' or 'down'")
                    .require(3, &Machine::is_function);

                auto code = vm -> get_int(1);
                std::string key;
                bool mouse;
                resolve_key(code, key, mouse);
                bool down;
                resolve_direction(vm -> get_string(2), down);
                auto ok = mouse ? bind_handler(mouse_binds, vm, code, down, 3) : bind_handler(key_binds, vm, code, down, 3);
                vm -> push_value(ok);
                return 1;
            });

            API::bind(vm, base_scope, "unbind", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(key, direction, exec)")
                    .require_enum(1, key_registry)
                    .require(2, &Machine::is_string)
                    .validate(2, [](Machine* vm, int idx) { return is_valid_direction(vm -> get_string(idx)); }, "direction must be either 'up' or 'down'")
                    .require(3, &Machine::is_function);

                auto code = vm -> get_int(1);
                std::string key;
                bool mouse;
                resolve_key(code, key, mouse);
                bool down;
                resolve_direction(vm -> get_string(2), down);
                auto ok = mouse ? unbind_handler(mouse_binds, vm, code, down, 3) : unbind_handler(key_binds, vm, code, down, 3);
                vm -> push_value(ok);
                return 1;
            });

            API::bind(vm, base_scope, "register", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, exec)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_function);

                auto name = vm -> get_string(1);
                auto ok = register_command(vm, name, 2);
                vm -> push_value(ok);
                return 1;
            });

            API::bind(vm, base_scope, "unregister", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, exec)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_function);

                auto name = vm -> get_string(1);
                auto ok = unregister_command(vm, name, 2);
                vm -> push_value(ok);
                return 1;
            });

            API::bind(vm, base_scope, "list", [](auto vm, auto& id) -> int {
                auto names = list_commands();
                vm -> create_table();
                for (int i = 0; i < (int)names.size(); ++i) {
                    vm -> push_value(names[i]);
                    vm -> set_table_field(i + 1, -2);
                }
                return 1;
            });

            API::bind(vm, base_scope, "execute", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, args)")
                    .require(1, &Machine::is_string)
                    .optional(2, &Machine::is_table);

                auto name = vm -> get_string(1);
                std::vector<std::string> args;
                if (vm -> get_count() >= 2 && vm -> is_table(2)) {
                    int len = vm -> get_length(2);
                    for (int i = 1; i <= len; ++i) {
                        vm -> get_table_field(i, 2);
                        args.push_back(vm -> to_string(-1));
                        vm -> pop(1);
                    }
                }
                auto ok = dispatch_command(vm, name, args);
                vm -> push_value(ok);
                return 1;
            });
        }

        static void inject(Machine* vm) {
            vm -> scope_set_enum(base_scope, "key", key_registry);
            vm -> scope_set_enum(base_scope, "cursor_mode", cursor_mode_registry);
        }

        static void clean(const std::string& env) {
            auto vm = Manager::Sandbox::get_singleton() -> get_vm();
            if (!vm) return;

            auto release_binds = [&](std::unordered_map<int, std::unordered_map<std::string, std::vector<BindHandler>>>& map) {
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
            release_binds(key_binds);
            release_binds(mouse_binds);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Input : vm_module {};
}
#endif