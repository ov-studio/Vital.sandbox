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
            inline static std::map<Timer*, bool> buffer = {};
        public:
            // Instantiators //
            inline static bool valid(Timer* identifier) {
                return buffer.find(identifier) != buffer.end();
            }

            inline Timer(std::function<void(Timer*)> exec, int interval = 0, int executions = 1) {
                buffer.emplace(this, true);
                Vital::Type::Thread([=](Vital::Type::Thread* thread) -> void {
                    int currentExecutions = 0;
                    int targetInterval = std::max(0, interval);
                    int targetExecutions = std::max(0, executions);
                    while (Timer::valid(this) && ((targetExecutions == 0) || (currentExecutions < targetExecutions))) {
                        thread -> sleep(interval);
                        currentExecutions++;
                        exec(this);
                    };
                    destroy();
                }).detach();
            }
    
            inline void destroy() {
                buffer.erase(this);
            }
    };
}