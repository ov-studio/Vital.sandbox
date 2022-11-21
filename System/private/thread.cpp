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
    bool isInstance(vital_thread* ref) { return instance[ref] ? ref : false; }
    create::create(std::function<void()> exec) {
        instance.emplace(this, true);
        thread = std::thread(exec);
    }
    create::~create() {
        instance.erase(this);
        thread.detach();
    }
}