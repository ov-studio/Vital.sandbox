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

namespace Vital::Type {
    std::map<Timer*, bool> Timer::buffer;

    // Instantiators //
    Timer::Timer(std::function<void(Timer*)> exec, int interval, int executions) {
        buffer.emplace(this, true);
        Vital::Type::Thread([=](Vital::Type::Thread* thread) -> void {
            int currentExecutions = 0;
            int targetInterval = std::max(0, interval), targetExecutions = std::max(0, executions);
            while (Timer::isInstance(this) && ((targetExecutions == 0) || (currentExecutions < targetExecutions))) {
                thread -> sleep(interval);
                currentExecutions++;
                exec(this);
            };
            destroy();
        }).detach();
    }
    void Timer::destroy() {
        buffer.erase(this);
    }

    // Utils //
    bool Timer::isInstance(Timer* identifier) {
        return buffer.find(identifier) != buffer.end();
    }
}