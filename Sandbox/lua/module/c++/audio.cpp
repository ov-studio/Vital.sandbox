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
    static bool isBound = false;
    Vital::System::Audio::Sound::vital_sound* fetchSound(void* userdata) {
        auto sound = static_cast<Vital::System::Audio::Sound::vital_sound*>(userdata);
        if (!Vital::System::Audio::Sound::isInstance(sound)) throw ErrorCode["invalid-entities"];
        return sound;
    }

    void vSandbox_Audio() {
        if (isBound) return; isBound = true;

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

        bind("sound", "isPlaying", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> isPlaying());
                return 1;
            });
        });

        bind("sound", "isPaused", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> isPaused());
                return 1;
            });
        });

        bind("sound", "isLooped", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> isLooped());
                return 1;
            });
        });

        bind("sound", "isVolumeRamped", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> isVolumeRamped());
                return 1;
            });
        });

        bind("sound", "isMuted", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> isMuted());
                return 1;
            });
        });

        bind("sound", "is3D", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> is3D());
                return 1;
            });
        });

        bind("sound", "setPriority", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                int value = vm -> getInt(2);
                vm -> setBool(sound -> setPriority(value));
                return 1;
            });
        });

        bind("sound", "setPaused", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isBool(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                int state = vm -> getBool(2);
                vm -> setBool(sound -> setPaused(state));
                return 1;
            });
        });

        bind("sound", "setLooped", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isBool(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                int state = vm -> getBool(2);
                vm -> setBool(sound -> setLooped(state));
                return 1;
            });
        });

        bind("sound", "setLoopCount", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                int value = vm -> getInt(2);
                vm -> setBool(sound -> setLoopCount(value));
                return 1;
            });
        });

        bind("sound", "setLoopPoint", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 3) || (!vm -> isUserData(1)) || (!vm -> isNumber(2)) || (!vm -> isNumber(3))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::LoopPoint point;
                point.start = vm -> getInt(2);
                point.end = vm -> getInt(3);
                vm -> setBool(sound -> setLoopPoint(point));
                return 1;
            });
        });

        bind("sound", "setPitch", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> setPitch(value));
                return 1;
            });
        });

        bind("sound", "setFrequency", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> setFrequency(value));
                return 1;
            });
        });

        bind("sound", "setVolume", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> setVolume(value));
                return 1;
            });
        });

        bind("sound", "setVolumeRamped", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isBool(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                bool state = vm -> getBool(2);
                vm -> setBool(sound -> setVolumeRamped(state));
                return 1;
            });
        });

        bind("sound", "setMuted", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isBool(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                bool state = vm -> getBool(2);
                vm -> setBool(sound -> setMuted(state));
                return 1;
            });
        });

        bind("sound", "setPosition", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                int value = vm -> getInt(2);
                vm -> setBool(sound -> setPosition(value));
                return 1;
            });
        });

        bind("sound", "setPan", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> setPan(value));
                return 1;
            });
        });

        bind("sound", "setMixInputLevels", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isTable(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::MixInputLevel level;
                level.count = vm -> getLength(2);
                if (level.count < 1) throw ErrorCode["invalid-arguments"];
                level.level = new float[(level.count)];
                try {
                    for(int i = 1; i <= level.count; i++) {
                        vm -> getTableField(i, 2);
                        if (!vm -> isNumber(-1)) throw ErrorCode["invalid-arguments"];
                        level.level[(i - 1)] = vm -> getFloat(-1);
                    }
                    vm -> setBool(sound -> setMixInputLevels(level));
                    delete[] level.level;
                }
                catch(...) {
                    if (level.level) delete[] level.level;
                    std::rethrow_exception(std::current_exception());
                }
                return 1;
            });
        });

        bind("sound", "setMixOutputLevels", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isTable(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::MixOutputLevel level;
                vm -> getTableField("frontLeft", 2);
                vm -> getTableField("frontRight", 2);
                vm -> getTableField("center", 2);
                vm -> getTableField("lowFrequency", 2);
                vm -> getTableField("surroundLeft", 2);
                vm -> getTableField("surroundRight", 2);
                vm -> getTableField("backLeft", 2);
                vm -> getTableField("backRight", 2);
                for(int i = -1; i >= -8; i--) {
                    if (!vm -> isNumber(i)) throw ErrorCode["invalid-arguments"];
                }
                level.frontLeft = vm -> getFloat(-8);
                level.frontRight = vm -> getFloat(-7);
                level.center = vm -> getFloat(-6);
                level.lowFrequency = vm -> getFloat(-5);
                level.surroundLeft = vm -> getFloat(-4);
                level.surroundRight = vm -> getFloat(-3);
                level.backLeft = vm -> getFloat(-2);
                level.backRight = vm -> getFloat(-1);
                vm -> setBool(sound -> setMixOutputLevels(level));
                return 1;
            });
        });

        bind("sound", "setMixMatrix", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isTable(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::MixMatrix matrix;
                matrix.countIn = vm -> getLength(2);
                if (matrix.countIn < 1) throw ErrorCode["invalid-arguments"];
                try {
                    for(int i = 1; i <= matrix.countIn; i++) {
                        vm -> getTableField(i, 2);
                        if (!vm -> isTable(-1)) throw ErrorCode["invalid-arguments"];
                        if (i == 1) {
                            matrix.countOut = vm -> getLength(-1);
                            if (matrix.countOut < 1) throw ErrorCode["invalid-arguments"];
                            matrix.matrix = new float[(matrix.countIn*matrix.countOut)];
                        }
                        for(int j = 1; j <= matrix.countOut; j++) {
                            vm -> getTableField(i, 3);
                            if (!vm -> isNumber(-1)) throw ErrorCode["invalid-arguments"];
                            matrix.matrix[((i - 1) + (j - 1))] = vm -> getFloat(-1);
                        }
                        vm -> pop(matrix.countOut + 1);
                    }
                    vm -> setBool(sound -> setMixMatrix(matrix));
                }
                catch(...) {
                    if (matrix.matrix) delete[] matrix.matrix;
                    std::rethrow_exception(std::current_exception());
                }
                return 1;
            });
        });

        bind("sound", "play", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> play());
                return 1;
            });
        });
        bind("sound", "stop", [](vital_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> stop());
                return 1;
            });
        });
    }
}