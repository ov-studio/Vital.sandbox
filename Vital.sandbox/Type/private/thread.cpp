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
    std::map<std::thread::id, vsdk_thread*> instance;

    // Instantiators //
    create::create(std::function<void(vsdk_thread*)> exec) {
        thread = std::thread([=]() -> void {
            std::thread::id id = std::this_thread::get_id();
            instance.emplace(id, this);
            exec(this);
        });
    }
    create::~create() {
        std::thread::id id = std::this_thread::get_id();
        instance.erase(id);
        if (thread.joinable()) detach();
    }
    vsdk_thread* fetchThread() {
        std::thread::id id = std::this_thread::get_id();
        return instance.find(id) != instance.end() ? instance.at(id) : nullptr;
    }

    // Utils //
    void create::sleep(int duration) {
        if (duration < 0) return;
        auto thread = fetchThread();
        if ((!thread) || (thread != this)) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    }
    void create::join() { thread.join(); }
    void create::detach() { thread.detach(); }
}