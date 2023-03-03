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

namespace Vital::Type::Thread {
    // Instantiators //
    std::map<std::thread::id, Vital::Type::Thread::Instance*> instance;
    Vital::Type::Thread::Instance* fetchThread() {
        std::thread::id threadID = std::this_thread::get_id();
        return instance.find(threadID) != instance.end() ? instance.at(threadID) : nullptr;
    }
    Instance::Instance(std::function<void(Vital::Type::Thread::Instance*)> exec) {
        thread = std::thread([=]() -> void {
            std::thread::id threadID = std::this_thread::get_id();
            instance.emplace(threadID, this);
            exec(this);
        });
    }
    Instance::~Instance() {
        std::thread::id threadID = std::this_thread::get_id();
        instance.erase(threadID);
        if (thread.joinable()) detach();
    }

    // Utils //
    void Instance::sleep(int duration) {
        if (duration < 0) return;
        auto thread = fetchThread();
        if ((!thread) || (thread != this)) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    }
    void Instance::join() { thread.join(); }
    void Instance::detach() { thread.detach(); }
}