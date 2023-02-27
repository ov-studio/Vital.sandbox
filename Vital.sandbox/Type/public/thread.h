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

namespace Vital::Type::Thread {
    class Instance {
        private:
            std::thread thread;
        public:
            // Instantiators //
            Instance(std::function<void(Vital::Type::Thread::Instance*)> exec);
            ~Instance();

            // Utils//
            void sleep(int duration);
            void join();
    };
    extern Vital::Type::Thread::Instance* fetchThread();
}