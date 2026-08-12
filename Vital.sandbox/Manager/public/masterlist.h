/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: masterlist.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Masterlist Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

// TODO: Improve
#pragma once
#include <Vital.sandbox/Engine/public/core.h>
#if !defined(VSDK_Client)
#include <Vital.sandbox/Config/server.h>


///////////////////////////////////
// Vital: Manager: Masterlist //
///////////////////////////////////

namespace Vital::Manager {
    // Reports this server's presence + live stats to the Vital.site
    // masterlist API on an interval, so it shows up in the public server
    // browser. Server-only -- does nothing on the client build.
    class Masterlist : public godot::Node, public Tool::Base<Masterlist> {
        friend class Tool::Base<Masterlist>;
        public:
            static constexpr const char* Name = "Masterlist.manager";
        private:
            const Config::Server* server_config = nullptr;
            Tool::Timer* timer = nullptr;
            bool active = false;
            std::mutex debounce_mutex;
            Tool::Timer* debounce_timer = nullptr;


            // Instantiators //
            Masterlist() = default;
            ~Masterlist() = default;

            
            // Internal //
            void send_heartbeat() const;
            void send_offline() const;
            static int get_interval_seconds();
            static int get_debounce_seconds();
        public:
            // Managers //
            void start(const Config::Server& config);
            void stop();
            void teardown();
            void refresh();

            // State //
            bool is_active() const;
    };
}
#endif