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
            std::atomic<bool> alive { true };
        public:
            inline Timer(std::function<void(Timer*)> exec, int interval = 0, int executions = 1) {
                Vital::Type::Thread([this, exec, interval, executions](Vital::Type::Thread* thread) {
                    int count_current = 0;
                    int count_target = std::max(0, executions);
                    int interval_target = std::max(0, interval);
                    while (valid() && (count_target == 0 || count_current < count_target)) {
                        thread -> sleep(interval_target);
                        ++count_current;
                        exec(this);
                    }
                    destroy();
                }).detach();
            }
    
            inline bool valid() {
                return alive.load(std::memory_order_acquire);
            }
        
            inline void destroy() {
                alive.store(false, std::memory_order_release);
            }
    };
}