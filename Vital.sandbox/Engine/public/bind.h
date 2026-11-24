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

// TODO: Improve
namespace Vital::Engine {
    // Engine key binds (console, main menu, screenshot). A bind is consumed once per physical press (latched), whether it arrives
    // through the Godot input event or through the keyboard poll: a focused webview can delay the key or drop it during a focus hand-off
    class Bind : public Tool::Base<Bind> {
        friend class Tool::Base<Bind>;
        public:
            static constexpr const char* Name = "Bind.engine";
        private:
            enum Type { console, mainmenu, screenshot, count };

            godot::Key keys[count] = {};
            bool latched[count] = {};
            double refresh_timer = 0.0;


            // Instantiators //
            Bind() = default;


            // Helpers //
            static int to_virtual_key(godot::Key key);   // Godot keycode -> Windows virtual key (0 when unknown or not on Windows)
            static int get_key_state(godot::Key key);    // 1 = down, 0 = up, -1 = the key can't be read from the keyboard (unknown key or not on Windows)
            static bool is_game_foreground();            // the game window (or one of its webviews) has the keyboard, not another application
            void refresh();
            bool is_available(int type);
            int find(godot::Key keycode);
            void run(int type);
        public:
            // Managers //
            bool handle_key(godot::InputEventKey* event);   // true when the key was consumed by a bind
            void poll(double delta);                        // reads the binds straight from the keyboard (Windows)
    };
}
#endif
