/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: timer.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Timer Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/timer.h>


/////////////////////////
// Vital: Type: Timer //
/////////////////////////

namespace Vital::Type::Timer {
    std::map<vsdk_timer*, bool> vsdk_timers;

    // Instantiators //
    create::create(std::function<void(create*)> exec, int interval, int executions) {
        vsdk_timers.emplace(this, true);
        Vital::Type::Thread::create([=](Vital::Type::Thread::create* thread) -> void {
            int currentExecutions = 0;
            int targetInterval = std::max(0, interval), targetExecutions = std::max(0, executions);
            while (isTimer(this) && ((targetExecutions == 0) || (currentExecutions < targetExecutions))) {
                thread -> sleep(interval);
                currentExecutions++;
                exec(this);
            };
            destroy();
        }).detach();
    }
    void create::destroy() {
        vsdk_timers.erase(this);
    }

    bool isTimer(vsdk_timer* identifier) {
        return vsdk_timers.find(identifier) != vsdk_timers.end();
    }
}