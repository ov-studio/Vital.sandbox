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

namespace Vital::Type {
    class Timer {
        private:
            static std::map<Timer*, bool> buffer;
        public:
            // Instantiators //
            Timer(std::function<void(Timer*)> exec, int interval = 0, int executions = 1);
            void destroy();

            // Utils //
            static bool valid(Timer*);
    };
}