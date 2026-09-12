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

#pragma once
#if !defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Config/server.h>


/////////////////////////////////
// Vital: Manager: Masterlist //
/////////////////////////////////

namespace Vital::Manager {
    class Masterlist : public godot::Node, public Tool::Base<Masterlist> {
        friend class Tool::Base<Masterlist>;
        public:
            static constexpr const char* Name = "Masterlist.manager";
        private:
            bool active = false;
            const Config::Server* server_config = nullptr;
            Tool::Timer* heartbeat_timer = nullptr;
            Tool::Timer* debounce_timer = nullptr;
            std::mutex debounce_mutex;


            // Instantiators //
            Masterlist() = default;
            ~Masterlist() = default;


            // Helpers //
            void send_heartbeat() const;
            void send_offline() const;
        public:
            // Managers //
            bool is_active() const;
            void start(const Config::Server& config);
            void stop();
            void teardown();
            void refresh();
    };
}
#endif