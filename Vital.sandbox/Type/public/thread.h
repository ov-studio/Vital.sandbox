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
            inline static std::mutex buffer_mutex;
            inline static std::unordered_map<std::thread::id, Thread*> buffer = {};
            std::thread thread;
            std::thread::id worker_id = {};
        public:
            // Instantiators //
            inline Thread(std::function<void(Thread*)> exec) {
                thread = std::thread([this, exec]() {
                    worker_id = std::this_thread::get_id();
                    {
                        std::lock_guard<std::mutex> lock(buffer_mutex);
                        buffer[worker_id] = this;
                    }
                    exec(this);
                    {
                        std::lock_guard<std::mutex> lock(buffer_mutex);
                        buffer.erase(worker_id);
                    }
                });
            }

            inline ~Thread() { if (thread.joinable()) thread.detach(); }
            inline void join() { if (thread.joinable()) thread.join(); }
            inline void detach() { if (thread.joinable()) thread.detach(); }

            inline void sleep(int duration) {
                if (duration < 0) return;
                Thread* current = fetch();
                if (current != this) return;
                std::this_thread::sleep_for(std::chrono::milliseconds(duration));
            }
    
            inline static Thread* fetch() {
                std::lock_guard<std::mutex> lock(buffer_mutex);
                auto it = buffer.find(std::this_thread::get_id());
                return it != buffer.end() ? it -> second : nullptr;
            }
        };
}