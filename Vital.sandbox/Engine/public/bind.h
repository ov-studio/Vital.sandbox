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
            static int to_virtual_key(godot::Key key);
            static int get_key_state(godot::Key key);
            static bool is_game_foreground();
            bool is_available(int type);
            int find(godot::Key keycode);
            void refresh();
            void run(int type);
        public:
            // Managers //
            bool handle_key(godot::InputEventKey* event);
            void poll(double delta);
    };
}
#endif
