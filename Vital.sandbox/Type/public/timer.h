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
            std::atomic<bool> alive{ true };
        public:
            Timer(std::function<void(Timer*)> exec, int interval = 0, int executions = 1) {
                std::thread([this, exec = std::move(exec), interval, executions]() {
                    int count = 0;
                    int limit = std::max(0, executions);
                    int delay = std::max(0, interval);
                    while (alive.load(std::memory_order_acquire) && (limit == 0 || count < limit)) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                        exec(this);
                        ++count;
                    }
                }).detach();
            }
            
            void stop() {
                alive.store(false, std::memory_order_release);
            }
    };
}