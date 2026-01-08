/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: thread.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Thread Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/index.h>


//////////////////////////
// Vital: Type: Thread //
//////////////////////////

namespace Vital::Type {
    class Thread {
        private:
            std::thread thread;
            std::thread::id worker_id = {};
            std::atomic<bool> alive { true };
        public:
            inline Thread(std::function<void(Thread*)> exec) {
                thread = std::thread([this, exec]() {
                    worker_id = std::this_thread::get_id();
                    exec(this);
                    alive.store(false, std::memory_order_release);
                });
            }
    
            inline ~Thread() {
                alive.store(false, std::memory_order_release);
                if (thread.joinable()) {
                    thread.detach();
                }
            }
    
            inline bool valid() const {
                return alive.load(std::memory_order_acquire);
            }
    
            inline void join() {
                if (thread.joinable()) {
                    thread.join();
                }
            }
    
            inline void detach() {
                if (thread.joinable()) {
                    thread.detach();
                }
            }
    
            inline void sleep(int duration) {
                if (duration < 0) return;
                if (!valid()) return;
                if (std::this_thread::get_id() != worker_id) return;
                std::this_thread::sleep_for(std::chrono::milliseconds(duration));
            }
        };
}