/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: thread.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Thread System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/vital.h>


////////////////////////////
// Vital: System: Thread //
////////////////////////////

namespace Vital::System::Thread {
    class create {
        private:
            std::thread thread;
        public:
            // Instantiators //
            create(std::function<void()> exec);
            ~create();

            // Utils//
            void join();
    };
    typedef create vital_thread;
    extern bool isInstance(vital_thread* ref);
}