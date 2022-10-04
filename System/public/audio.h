/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: audio.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Audio System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/vital.h>
#include <Vendor/fmod/include/fmod.hpp>
#include <Vendor/fmod/include/fmod_errors.h>


///////////////////////////
// Vital: System: Audio //
///////////////////////////

namespace Vital::System::Audio {
    extern bool create();
    extern bool destroy();
    extern bool update();
    extern FMOD::Sound* play(std::string& path);

    namespace Sound {
        class create {
            private:
                FMOD::Sound* sound = nullptr;
                FMOD::Channel* channel = nullptr;
            public:
                // Instantiators //
                create(std::string& path);
                bool destroy();

                void play();
                void play(FMOD::ChannelGroup* channelGroup);
                void setChannelGroup(FMOD::ChannelGroup* channelGroup);
        };
        typedef create vital_sound;
    }
}