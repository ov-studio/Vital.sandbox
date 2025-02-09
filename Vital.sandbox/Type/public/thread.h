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
    class create {
        private:
            std::thread thread;
        public:
            // Instantiators //
            create(std::function<void(Vital::Type::Thread::create*)> exec);
            ~create();

            // Utils//
            void sleep(int duration);
            void join();
            void detach();
    };
    typedef create vsdk_thread;
    extern vsdk_thread* fetchThread();
}