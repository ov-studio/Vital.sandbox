/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: thread.h
     Author: ov-studio
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
            inline static std::mutex mutex;
            inline static std::unordered_map<std::thread::id, Thread*> buffer;
            inline static std::unordered_set<Thread*> live;
            std::thread thread;

            Thread(std::function<void(Thread*)> exec) {
                thread = std::thread([this, exec = std::move(exec)]() {
                    std::thread::id id = std::this_thread::get_id();
                    {
                        std::lock_guard<std::mutex> lock(mutex);
                        buffer.emplace(id, this);
                    }
                    exec(this);
                    {
                        std::lock_guard<std::mutex> lock(mutex);
                        buffer.erase(std::this_thread::get_id());
                        live.erase(this);
                    }
                    delete this;
                });
            }

        public:
            static Thread* create(std::function<void(Thread*)> exec) {
                return new Thread(std::move(exec));
            }

            ~Thread() {
                if (thread.joinable()) thread.detach();
            }

            static Thread* fetch() {
                std::thread::id id = std::this_thread::get_id();
                std::lock_guard<std::mutex> lock(mutex);
                auto it = buffer.find(id);
                return it != buffer.end() ? it -> second : nullptr;
            }

            static bool valid(Thread* instance) {
                std::lock_guard<std::mutex> lock(mutex);
                return live.count(instance) > 0;
            }

            void join() {
                if (!valid(this)) return;
                if (thread.joinable()) thread.join();
            }

            void detach() {
                if (!valid(this)) return;
                if (thread.joinable()) thread.detach();
            }

            void sleep(int duration) {
                if (duration < 0) return;
                if (fetch() != this) return;
                std::this_thread::sleep_for(std::chrono::milliseconds(duration));
            }
    };
}