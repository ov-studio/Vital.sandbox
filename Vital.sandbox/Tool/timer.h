/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: timer.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Timer Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <Vital.sandbox/Tool/thread.h>


/////////////////////////
// Vital: Tool: Timer //
/////////////////////////

namespace Vital::Tool {
    class Timer {
        private:
            inline static std::map<Timer*, bool> buffer;
            inline static std::mutex mutex;
            
            Timer(std::function<void(Timer*)> exec, int interval, int executions) {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    buffer.emplace(this, true);
                }
                interval = std::max(0, interval);
                executions = std::max(0, executions);
                Thread::create([=](Thread* thread) {
                    int count = 0;
                    while (valid(this) && ((executions == 0) || (count < executions))) {
                        thread -> sleep(interval);
                        if (!valid(this)) break;
                        count++;
                        exec(this);
                    }
                    if (valid(this)) stop();
                }) -> detach();
            }
        public:
            static Timer* create(std::function<void(Timer*)> exec, int interval = 0, int executions = 1) {
                return new Timer(std::move(exec), interval, executions);
            }

            static bool valid(Timer* identifier) {
                std::lock_guard<std::mutex> lock(mutex);
                return buffer.find(identifier) != buffer.end();
            }

            void stop() {
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    buffer.erase(this);
                }
                delete this;
            }
    };
}