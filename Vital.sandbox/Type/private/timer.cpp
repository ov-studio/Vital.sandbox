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
    // Instantiators //
    Instance::Instance(std::function<void(Vital::Type::Timer::Instance*)> exec, int interval, int executions) {
        targetInterval = interval, targetExecutions = executions;
        Vital::Type::Thread::Instance([=](Vital::Type::Thread::Instance* thread) -> void {
            while (!isUnloaded && ((targetExecutions == 0) || (currentExecutions < targetExecutions))) {
                thread -> sleep(interval);
                currentExecutions++;
                exec(this);
            }
            delete this;
        });
    }
    Instance::~Instance() {
        isUnloaded = true;
        delete thread;
    }
}