/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: thread.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Thread Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/thread.h>


//////////////////////////
// Vital: Type: Thread //
//////////////////////////

namespace Vital::Type {
    std::map<std::thread::id, Thread*> Thread::buffer;

    // Instantiators //
    Thread::Thread(std::function<void(Thread*)> exec) {
        thread = std::thread([=]() -> void {
            std::thread::id id = std::this_thread::get_id();
            buffer.emplace(id, this);
            exec(this);
        });
    }
    Thread::~Thread() {
        std::thread::id id = std::this_thread::get_id();
        buffer.erase(id);
        if (thread.joinable()) detach();
    }

    // Utils //
    void Thread::sleep(int duration) {
        if (duration < 0) return;
        auto thread = fetchThread();
        if ((!thread) || (thread != this)) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    }
    void Thread::join() { thread.join(); }
    void Thread::detach() { thread.detach(); }
    Thread* Thread::fetchThread() {
        std::thread::id id = std::this_thread::get_id();
        return buffer.find(id) != buffer.end() ? buffer.at(id) : nullptr;
    }
}