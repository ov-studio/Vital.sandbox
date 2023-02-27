/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: private: timer.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Timer System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/timer.h>


///////////////////////////
// Vital: System: Timer //
///////////////////////////

namespace Vital::System::Timer {
    // Instantiators //
    std::map<vital_timer*, bool> instance;
    bool isInstance(vital_timer* ref) { return instance.find(ref) != instance.end(); }
    create::create(std::function<void()> exec, int interval, int executions) {
        instance.emplace(this, true);
        targetInterval = interval;
        currentExecutions = 0, targetExecutions = executions;
        auto self = this;
        thread = Vital::System::Thread::create([=]() -> void {
            while ((self.targetExecutions == 0) or (self.currentExecutions < self.executions)) {
                this -> sleep(interval);
                self.executions++;
                exec();
            }
            delete self;
        });
    }
    create::~create() {
        instance.erase(this);
        std::cout << "Timer destroyed";
    }
}