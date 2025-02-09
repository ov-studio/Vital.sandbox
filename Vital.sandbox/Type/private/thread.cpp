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
    std::map<std::thread::id, vsdk_thread*> vsdk_threads;

    // Instantiators //
    create::create(std::function<void(vsdk_thread*)> exec) {
        thread = std::thread([=]() -> void {
            std::thread::id id = std::this_thread::get_id();
            vsdk_threads.emplace(id, this);
            exec(this);
        });
    }
    create::~create() {
        std::thread::id id = std::this_thread::get_id();
        vsdk_threads.erase(id);
        if (thread.joinable()) detach();
    }
    vsdk_thread* fetchThread() {
        std::thread::id id = std::this_thread::get_id();
        return vsdk_threads.find(id) != vsdk_threads.end() ? vsdk_threads.at(id) : nullptr;
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