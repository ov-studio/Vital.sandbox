/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: public: thread.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Thread Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>


//////////////////////////
// Vital: Tool: Thread //
//////////////////////////

namespace Vital::Tool {
    class Thread {
        private:
            static inline std::unordered_map<std::thread::id, Thread*> buffer;
            static inline std::mutex mutex;
            std::thread thread;
        public:
            Thread(std::function<void(Thread*)> exec) {
                thread = std::thread([this, exec = std::move(exec)]() {
                    std::thread::id id = std::this_thread::get_id();
                    {
                        std::lock_guard<std::mutex> lock(mutex);
                        buffer.emplace(id, this);
                    }
                    exec(this);
                });
            }
        
            ~Thread() {
                std::thread::id id = std::this_thread::get_id();
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    buffer.erase(id);
                }
                detach();
            }
        
            static Thread* fetch() {
                std::thread::id id = std::this_thread::get_id();
                std::lock_guard<std::mutex> lock(mutex);
                auto it = buffer.find(id);
                return it != buffer.end() ? it -> second : nullptr;
            }
    
            void join() { 
                if (thread.joinable()) thread.join(); 
            }
        
            void detach() { 
                if (thread.joinable()) thread.detach(); 
            }
        
            void sleep(int duration) {
                if (duration < 0) return;
                if (fetch() != this) return;
                std::this_thread::sleep_for(std::chrono::milliseconds(duration));
            }
    };
}