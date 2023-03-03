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
    std::map<Instance*, bool> instance;
    bool isInstance(Instance* identifier) { return instance.find(identifier) != instance.end(); }
    Instance::Instance(std::function<void(Vital::Type::Timer::Instance*)> exec, int interval, int executions) {
        instance.emplace(this, true);
        Vital::Type::Thread::Instance([=](Vital::Type::Thread::Instance* thread) -> void {
            int currentExecutions = 0;
            int targetInterval = std::max(0, interval), targetExecutions = std::max(0, executions);
            std::cout << targetExecutions << " : " << currentExecutions;
            while (isInstance(this) && ((targetExecutions == 0) || (currentExecutions < targetExecutions))) {
                thread -> sleep(interval);
                currentExecutions++;
                exec(this);
            };
            destroy();
        }).detach();
    }
    void Instance::destroy() { instance.erase(this); }
}