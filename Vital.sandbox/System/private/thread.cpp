/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: private: thread.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Thread System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/thread.h>


////////////////////////////
// Vital: System: Thread //
////////////////////////////

namespace Vital::System::Thread {
    // Instantiators //
    std::map<vital_thread*, bool> instance;
    std::map<std::thread::id, vital_thread*> instance_ref;
    bool isInstance(vital_thread* ref) { return instance.find(ref) != instance.end(); }
    vital_thread* fetchThread() {
        std::thread::id threadID = std::this_thread::get_id();
        return instance_ref.find(threadID) != instance_ref.end() ? instance_ref.at(threadID) : nullptr;
    }
    create::create(std::function<void()> exec) {
        instance.emplace(this, true);
        thread = std::thread(exec);
    }
    create::~create() {
        std::thread::id threadID = std::this_thread::get_id();
        instance.erase(this);
        instance_ref.erase(threadID);
        if (thread.joinable()) thread.detach();
    }

    // Utils //
    void create::sleep(int duration) {
        if (duration < 0) return;
        auto thread = fetchThread();
        if (!thread || (thread != this)) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    }
    void create::join() { thread.join(); }
}