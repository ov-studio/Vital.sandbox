/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: timer.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Timer Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/index.h>
#include <Type/public/thread.h>


/////////////////////////
// Vital: Type: Timer //
/////////////////////////

namespace Vital::Type::Timer {
    class Instance {
        private:
            bool isUnloaded = false;
            int currentExecutions = 0;
            int targetInterval, targetExecutions;
        public:
            // Instantiators //
            Instance(std::function<void(Vital::Type::Timer::Instance*)> exec, int interval = 0, int executions = 1);
            ~Instance();
    };
}