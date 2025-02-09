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
            static std::map<std::thread::id, Thread*> buffer;
            std::thread thread;
        public:
            // Instantiators //
            Thread(std::function<void(Thread*)> exec);
            ~Thread();

            // Utils//
            void join();
            void detach();
            void sleep(int duration);
            static Thread* fetchThread();
    };
}