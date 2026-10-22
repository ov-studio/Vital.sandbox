/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: mainmenu.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: MainMenu Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/webview.h>


//////////////////////////////
// Vital: Engine: MainMenu //
//////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    class MainMenu : public godot::Control, public Tool::Base<MainMenu> {
        friend class Tool::Base<MainMenu>;
        private:
            Webview* webview = nullptr;
            std::atomic<bool> webview_ready { false };
            static std::atomic<float> draw_distance_mult;
            static std::atomic<float> particle_distance_mult;
            static std::atomic<float> mouse_sens_horizontal;
            static std::atomic<float> mouse_sens_vertical;
            std::atomic<bool> scan_running { false };
            std::atomic<bool> masterlist_running { false };


            // Instantiators //
            MainMenu();
            ~MainMenu();
        public:
            static constexpr const char* Name = "MainMenu.engine";


            // Managers //
            bool is_ready();
            bool is_visible();
            void ready();
            void check_updates();
            void scan_local_servers();
            void fetch_masterlist();
            void send_connection();
            void show();
            void hide();
            void toggle();

            // Client draw-distance multiplier (1.0 = Lua value as-is).
            // set_far_clip applies it; get_far_clip divides it out.
            static float get_draw_distance_mult();
            static void set_draw_distance_mult(float mult);

            // Client particle-distance multiplier on the active camera far plane (1.0 = no extra limit).
            // GPU_Particles reads it every frame, so nothing needs pushing when it changes.
            static float get_particle_distance_mult();
            static void set_particle_distance_mult(float mult);

            // Client mouse look multipliers (1.0 = default). Games read these when applying look.
            static float get_mouse_sens_horizontal();
            static void set_mouse_sens_horizontal(float mult);
            static float get_mouse_sens_vertical();
            static void set_mouse_sens_vertical(float mult);


            // Events //
            void on_message(godot::String message);
    };
}
#endif
