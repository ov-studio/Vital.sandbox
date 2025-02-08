/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: module: c++: audio.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
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

        bind("sound", "create", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw ErrorCode["invalid-arguments"];
                std::string path = vm -> getString(1);
                if (!Vital::System::File::exists(path)) throw ErrorCode["file-nonexistent"];
                vm -> setUserData(static_cast<void*>(new Vital::System::Audio::Sound::create(path)));
                return 1;
            });
        });

        bind("sound", "destroy", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                delete sound;
                vm -> setBool(true);
                return 1;
            });
        });

        bind("sound", "isPlaying", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> isPlaying());
                return 1;
            });
        });

        bind("sound", "isPaused", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> isPaused());
                return 1;
            });
        });

        bind("sound", "isLooped", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> isLooped());
                return 1;
            });
        });

        bind("sound", "isVolumeRamped", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> isVolumeRamped());
                return 1;
            });
        });

        bind("sound", "isMuted", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> isMuted());
                return 1;
            });
        });

        bind("sound", "is3D", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> is3D());
                return 1;
            });
        });

        bind("sound", "setPriority", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                int value = vm -> getInt(2);
                vm -> setBool(sound -> setPriority(value));
                return 1;
            });
        });

        bind("sound", "setPaused", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isBool(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                int state = vm -> getBool(2);
                vm -> setBool(sound -> setPaused(state));
                return 1;
            });
        });

        bind("sound", "setLooped", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isBool(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                int state = vm -> getBool(2);
                vm -> setBool(sound -> setLooped(state));
                return 1;
            });
        });

        bind("sound", "setLoopCount", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                int value = vm -> getInt(2);
                vm -> setBool(sound -> setLoopCount(value));
                return 1;
            });
        });

        bind("sound", "setLoopPoint", [](vsdk_ref* ref) -> int {
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

        bind("sound", "setPitch", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> setPitch(value));
                return 1;
            });
        });

        bind("sound", "setFrequency", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> setFrequency(value));
                return 1;
            });
        });

        bind("sound", "setVolume", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> setVolume(value));
                return 1;
            });
        });

        bind("sound", "setVolumeRamped", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isBool(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                bool state = vm -> getBool(2);
                vm -> setBool(sound -> setVolumeRamped(state));
                return 1;
            });
        });

        bind("sound", "setMuted", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isBool(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                bool state = vm -> getBool(2);
                vm -> setBool(sound -> setMuted(state));
                return 1;
            });
        });

        bind("sound", "setPosition", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                int value = vm -> getInt(2);
                vm -> setBool(sound -> setPosition(value));
                return 1;
            });
        });

        bind("sound", "setPan", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> setPan(value));
                return 1;
            });
        });

        bind("sound", "setMixInputLevels", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isTable(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::MixInputLevel level;
                level.count = vm -> getLength(2);
                if (level.count < 1) throw ErrorCode["invalid-arguments"];
                level.level = new float[(level.count - 1)];
                try {
                    for(int i = 1; i <= level.count; i++) {
                        vm -> getTableField(i, 2);
                        if (!vm -> isNumber(-1)) throw ErrorCode["invalid-arguments"];
                        int index = i - 1;
                        level.level[index] = vm -> getFloat(-1);
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

        bind("sound", "setMixOutputLevels", [](vsdk_ref* ref) -> int {
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

        bind("sound", "setMixMatrix", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isTable(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::MixMatrix matrix;
                matrix.countOut = vm -> getLength(2);
                if (matrix.countOut < 1) throw ErrorCode["invalid-arguments"];
                try {
                    for(int i = 1; i <= matrix.countOut; i++) {
                        vm -> getTableField(i, 2);
                        if (!vm -> isTable(-1)) throw ErrorCode["invalid-arguments"];
                        if (i == 1) {
                            matrix.countIn = vm -> getLength(-1);
                            if (matrix.countIn < 1) throw ErrorCode["invalid-arguments"];
                            int size = matrix.countIn*matrix.countOut;
                            matrix.matrix = new float[(size - 1)];
                        }
                        for(int j = 1; j <= matrix.countIn; j++) {
                            vm -> getTableField(i, 3);
                            if (!vm -> isNumber(-1)) throw ErrorCode["invalid-arguments"];
                            int index = (i - 1) + (j - 1);
                            matrix.matrix[index] = vm -> getFloat(-1);
                        }
                        vm -> pop(matrix.countIn + 1);
                    }
                    vm -> setBool(sound -> setMixMatrix(matrix));
                    delete[] matrix.matrix;
                }
                catch(...) {
                    if (matrix.matrix) delete[] matrix.matrix;
                    std::rethrow_exception(std::current_exception());
                }
                return 1;
            });
        });

        bind("sound", "set3D", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isBool(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                bool state = vm -> getBool(2);
                vm -> setBool(sound -> set3D(state));
                return 1;
            });
        });

        bind("sound", "set3DAttributes", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 3) || (!vm -> isUserData(1)) || (!vm -> isTable(2)) || (!vm -> isTable(3))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Math::Vector3D position, velocity;
                vm -> getTableField("x", 2);
                vm -> getTableField("y", 2);
                vm -> getTableField("z", 2);
                vm -> getTableField("x", 3);
                vm -> getTableField("y", 3);
                vm -> getTableField("z", 3);
                for(int i = -1; i >= -6; i--) {
                    if (!vm -> isNumber(i)) throw ErrorCode["invalid-arguments"];
                }
                position.x = vm -> getFloat(-6);
                position.y = vm -> getFloat(-5);
                position.z = vm -> getFloat(-4);
                velocity.x = vm -> getFloat(-3);
                velocity.y = vm -> getFloat(-2);
                velocity.z = vm -> getFloat(-1);
                vm -> setBool(sound -> set3DAttributes(position, velocity));
                return 1;
            });
        });

        bind("sound", "set3DConeSettings", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isTable(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::ConeSetting3D setting;
                vm -> getTableField("insideAngle", 2);
                vm -> getTableField("outsideAngle", 2);
                vm -> getTableField("outsideVolume", 2);
                for(int i = -1; i >= -3; i--) {
                    if (!vm -> isNumber(i)) throw ErrorCode["invalid-arguments"];
                }
                setting.insideAngle = vm -> getFloat(-3);
                setting.outsideAngle = vm -> getFloat(-2);
                setting.outsideVolume = vm -> getFloat(-1);
                vm -> setBool(sound -> set3DConeSettings(setting));
                return 1;
            });
        });

        bind("sound", "set3DConeOrientation", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isTable(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Math::Vector3D orientation;
                vm -> getTableField("x", 2);
                vm -> getTableField("y", 2);
                vm -> getTableField("z", 2);
                for(int i = -1; i >= -3; i--) {
                    if (!vm -> isNumber(i)) throw ErrorCode["invalid-arguments"];
                }
                orientation.x = vm -> getFloat(-3);
                orientation.y = vm -> getFloat(-2);
                orientation.z = vm -> getFloat(-1);
                vm -> setBool(sound -> set3DConeOrientation(orientation));
                return 1;
            });
        });

        bind("sound", "set3DDistanceFilter", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 3) || (!vm -> isUserData(1)) || (!vm -> isBool(2)) || (!vm -> isTable(3))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::DistanceFilter3D filter;
                vm -> getTableField("customLevel", 3);
                vm -> getTableField("centerFrequency", 3);
                for(int i = -1; i >= -2; i--) {
                    if (!vm -> isNumber(i)) throw ErrorCode["invalid-arguments"];
                }
                filter.isEnabled = vm -> getBool(2);
                filter.customLevel = vm -> getFloat(-2);
                filter.centerFrequency = vm -> getFloat(-1);
                vm -> setBool(sound -> set3DDistanceFilter(filter));
                return 1;
            });
        });

        bind("sound", "set3DDopplerLevel", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> set3DDopplerLevel(value));
                return 1;
            });
        });

        bind("sound", "set3DLevel", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> set3DLevel(value));
                return 1;
            });
        });

        bind("sound", "set3DRange", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 3) || (!vm -> isUserData(1)) || (!vm -> isNumber(2)) || (!vm -> isNumber(3))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::Range3D range;
                range.minDistance = vm -> getFloat(2);
                range.maxDistance = vm -> getFloat(3);
                vm -> setBool(sound -> set3DRange(range));
                return 1;
            });
        });

        bind("sound", "set3DOcclusion", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isTable(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::Occlusion3D occlusion;
                vm -> getTableField("directOcclusion", 2);
                vm -> getTableField("reverbOcclusion", 2);
                for(int i = -1; i >= -2; i--) {
                    if (!vm -> isNumber(i)) throw ErrorCode["invalid-arguments"];
                }
                occlusion.directOcclusion = vm -> getFloat(-2);
                occlusion.reverbOcclusion = vm -> getFloat(-1);
                vm -> setBool(sound -> set3DOcclusion(occlusion));
                return 1;
            });
        });


        bind("sound", "set3DSpread", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> set3DSpread(value));
                return 1;
            });
        });

        bind("sound", "play", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> play());
                return 1;
            });
        });
    
        bind("sound", "stop", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> stop());
                return 1;
            });
        });

        bind("sound", "getPriority", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> getPriority());
                return 1;
            });
        });

        bind("sound", "getLoopCount", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> getLoopCount());
                return 1;
            });
        });

        bind("sound", "getLoopPoint", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::LoopPoint point;
                sound -> getLoopPoint(point);
                vm -> setNumber(static_cast<int>(point.start));
                vm -> setNumber(static_cast<int>(point.end));
                return 2;
            });
        });

        bind("sound", "getPitch", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> getPitch());
                return 1;
            });
        });

        bind("sound", "getFrequency", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> getFrequency());
                return 1;
            });
        });

        bind("sound", "getVolume", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> getVolume());
                return 1;
            });
        });

        bind("sound", "getAudibility", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> getAudibility());
                return 1;
            });
        });

        bind("sound", "getPosition", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(static_cast<int>(sound -> getPosition()));
                return 1;
            });
        });

        bind("sound", "getMixMatrix", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::MixMatrix matrix;
                sound -> getMixMatrix(matrix);
                vm -> createTable();
                for (int i = 1; i <= matrix.countOut; i++) {
                    vm -> createTable();
                    for (int j = 1; j <= matrix.countIn; j++) {
                        vm -> pushNumber(matrix.matrix[(i + j)]);
                    }
                    vm -> pushTable();
                }
                return 1;
            });
        });

        bind("sound", "get3DAttributes", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Math::Vector3D position, velocity;
                sound -> get3DAttributes(position, velocity);
                vm -> createTable();
                vm -> registerNumber("x", position.x);
                vm -> registerNumber("y", position.y);
                vm -> registerNumber("z", position.z);
                vm -> createTable();
                vm -> registerNumber("x", velocity.x);
                vm -> registerNumber("y", velocity.y);
                vm -> registerNumber("z", velocity.z);
                return 2;
            });
        });

        bind("sound", "get3DConeSettings", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::ConeSetting3D setting;
                sound -> get3DConeSettings(setting);
                vm -> createTable();
                vm -> registerNumber("insideAngle", setting.insideAngle);
                vm -> registerNumber("outsideAngle", setting.outsideAngle);
                vm -> registerNumber("outsideVolume", setting.outsideVolume);
                return 1;
            });
        });

        bind("sound", "get3DConeOrientation", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Math::Vector3D orientation;
                sound -> get3DConeOrientation(orientation);
                vm -> createTable();
                vm -> registerNumber("x", orientation.x);
                vm -> registerNumber("y", orientation.y);
                vm -> registerNumber("z", orientation.z);
                return 1;
            });
        });

        bind("sound", "get3DDistanceFilter", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::DistanceFilter3D filter;
                sound -> get3DDistanceFilter(filter);
                vm -> setBool(filter.isEnabled);
                vm -> createTable();
                vm -> registerNumber("customLevel", filter.customLevel);
                vm -> registerNumber("centerFrequency", filter.centerFrequency);
                return 2;
            });
        });

        bind("sound", "get3DDopplerLevel", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> get3DDopplerLevel());
                return 1;
            });
        });
    
        bind("sound", "get3DLevel", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> get3DLevel());
                return 1;
            });
        });
    
        bind("sound", "get3DRange", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::Range3D range;
                sound -> get3DRange(range);
                vm -> setNumber(range.minDistance);
                vm -> setNumber(range.maxDistance);
                return 2;
            });
        });
    
        bind("sound", "get3DOcclusion", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::Occlusion3D occlusion;
                sound -> get3DOcclusion(occlusion);
                vm -> createTable();
                vm -> registerNumber("directOcclusion", occlusion.directOcclusion);
                vm -> registerNumber("reverbOcclusion", occlusion.reverbOcclusion);
                return 1;
            });
        });

        bind("sound", "get3DSpread", [](vsdk_ref* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw ErrorCode["invalid-arguments"];
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> get3DSpread());
                return 1;
            });
        });
    }
}