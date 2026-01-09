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
#include <Vital.sandbox/Type/public/index.h>
#include <Vital.sandbox/Type/public/thread.h>


/////////////////////////
// Vital: Type: Timer //
/////////////////////////

namespace Vital::Type {
    class Timer {
        private:
            static inline std::map<Timer*, bool> buffer;
            static inline std::mutex mutex;
        public:
            Timer(std::function<void(Timer*)> exec, int interval = 0, int executions = 1) {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    buffer.emplace(this, true);
                }
                Thread([=](Thread* thread) {
                    int currentExecutions = 0;
                    int targetInterval = std::max(0, interval);
                    int targetExecutions = std::max(0, executions);
        
                    while (valid(this) && ((targetExecutions == 0) || (currentExecutions < targetExecutions))) {
                        thread->sleep(targetInterval);
                        currentExecutions++;
                        exec(this);
                    }
                    stop();
                }).detach();
            }
        
            static bool valid(Timer* identifier) {
                std::lock_guard<std::mutex> lock(mutex);
                return buffer.find(identifier) != buffer.end();
            }
    
            void stop() {
                std::lock_guard<std::mutex> lock(mutex);
                buffer.erase(this);
            }
    };
}