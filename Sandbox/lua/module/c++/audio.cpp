/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: module: c++: audio.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Audio Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/file.h>
#include <System/public/audio.h>
#include <Sandbox/lua/public/api.h>


////////////////////////
// Lua: Audio Binder //
////////////////////////

namespace Vital::Sandbox::Lua::API {
    //const bool isInstance()
    void vSandbox_Audio() {
        bind("sound", "create", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw ErrorCode["invalid-arguments"];
                std::string path = vm -> getString(1);
                if (!Vital::System::File::exists(path)) throw ErrorCode["file-nonexistent"];
                vm -> setUserData(static_cast<void*>(new Vital::System::Audio::Sound::create(path)));
                return 1;
            });
        });

        bind("sound", "play", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = static_cast<Vital::System::Audio::Sound::vital_sound*>(vm -> getUserData(1));
                if (!Vital::System::Audio::Sound::isInstance(sound)) throw ErrorCode["invalid-entities"];
                sound -> play();
                vm -> setBool(true);
                return 1;
            });
        });
    }
}