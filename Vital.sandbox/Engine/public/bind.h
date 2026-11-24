/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: bind.h
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
#include <Vital.sandbox/Engine/public/core.h>


//////////////////////////
// Vital: Engine: Bind //
//////////////////////////

namespace Vital::Engine {
    class Bind : public Tool::Base<Bind> {
        friend class Tool::Base<Bind>;
        public:
            static constexpr const char* Name = "Bind.engine";
        private:
            enum class Type {
                Console,
                MainMenu,
                Screenshot
            };

            struct Native {
                godot::Key key;
                int code;
            };

            struct Slot {
                Type type;
                const char* config;
                godot::Key key = godot::Key::KEY_NONE;
                bool latched = false;
            };

            std::array<Slot, 3> slots = {{
                { Type::Console,    "config/console"    },
                { Type::MainMenu,   "config/mainmenu"   },
                { Type::Screenshot, "config/screenshot" }
            }};
            double refresh_timer = 0.0;


            // Instantiators //
            Bind() = default;


            // Helpers //
            static int to_virtual_key(godot::Key key);
            static int get_key_state(godot::Key key);
            static int get_x11_key_state(int keysym);
            static bool is_game_foreground();
            bool is_available(const Slot& slot);
            Slot* find(godot::Key keycode);
            void refresh();
            void run(const Slot& slot);
        public:
            // Managers //
            bool handle_key(godot::InputEventKey* event);
            void poll(double delta);
    };
}
#endif
