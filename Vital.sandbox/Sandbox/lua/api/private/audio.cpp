/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: private: audio.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Audio APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Sandbox/lua/api/public/audio.h>
#include <System/public/file.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Sandbox::Lua::API {
    void Audio::bind(void* instance) {
        auto vm = static_cast<vsdk_vm*>(instance);
    
        API::bind(vm, "sound", "create", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                std::string path = vm -> getString(1);
                if (!Vital::System::File::exists(path)) throw std::runtime_error(ErrorCode["file-nonexistent"]);
                vm -> setUserData(static_cast<void*>(new Vital::System::Audio::Sound::create(path)));
                return 1;
            });
        });

        API::bind(vm, "sound", "destroy", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                delete sound;
                vm -> setBool(true);
                return 1;
            });
        });

        API::bind(vm, "sound", "isPlaying", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> isPlaying());
                return 1;
            });
        });

        API::bind(vm, "sound", "isPaused", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> isPaused());
                return 1;
            });
        });

        API::bind(vm, "sound", "isLooped", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> isLooped());
                return 1;
            });
        });

        API::bind(vm, "sound", "isVolumeRamped", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> isVolumeRamped());
                return 1;
            });
        });

        API::bind(vm, "sound", "isMuted", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> isMuted());
                return 1;
            });
        });

        API::bind(vm, "sound", "is3D", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> is3D());
                return 1;
            });
        });

        API::bind(vm, "sound", "setPriority", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                int value = vm -> getInt(2);
                vm -> setBool(sound -> setPriority(value));
                return 1;
            });
        });

        API::bind(vm, "sound", "setPaused", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isBool(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                int state = vm -> getBool(2);
                vm -> setBool(sound -> setPaused(state));
                return 1;
            });
        });

        API::bind(vm, "sound", "setLooped", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isBool(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                int state = vm -> getBool(2);
                vm -> setBool(sound -> setLooped(state));
                return 1;
            });
        });

        API::bind(vm, "sound", "setLoopCount", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                int value = vm -> getInt(2);
                vm -> setBool(sound -> setLoopCount(value));
                return 1;
            });
        });

        API::bind(vm, "sound", "setLoopPoint", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 3) || (!vm -> isUserData(1)) || (!vm -> isNumber(2)) || (!vm -> isNumber(3))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::LoopPoint point;
                point.start = vm -> getInt(2);
                point.end = vm -> getInt(3);
                vm -> setBool(sound -> setLoopPoint(point));
                return 1;
            });
        });

        API::bind(vm, "sound", "setPitch", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> setPitch(value));
                return 1;
            });
        });

        API::bind(vm, "sound", "setFrequency", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> setFrequency(value));
                return 1;
            });
        });

        API::bind(vm, "sound", "setVolume", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> setVolume(value));
                return 1;
            });
        });

        API::bind(vm, "sound", "setVolumeRamped", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isBool(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                bool state = vm -> getBool(2);
                vm -> setBool(sound -> setVolumeRamped(state));
                return 1;
            });
        });

        API::bind(vm, "sound", "setMuted", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isBool(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                bool state = vm -> getBool(2);
                vm -> setBool(sound -> setMuted(state));
                return 1;
            });
        });

        API::bind(vm, "sound", "setPosition", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                int value = vm -> getInt(2);
                vm -> setBool(sound -> setPosition(value));
                return 1;
            });
        });

        API::bind(vm, "sound", "setPan", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> setPan(value));
                return 1;
            });
        });

        API::bind(vm, "sound", "setMixInputLevels", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isTable(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::MixInputLevel level;
                level.count = vm -> getLength(2);
                if (level.count < 1) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                level.level = new float[(level.count - 1)];
                try {
                    for(int i = 1; i <= level.count; i++) {
                        vm -> getTableField(i, 2);
                        if (!vm -> isNumber(-1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
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

        API::bind(vm, "sound", "setMixOutputLevels", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isTable(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
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
                    if (!vm -> isNumber(i)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
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

        API::bind(vm, "sound", "setMixMatrix", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isTable(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::MixMatrix matrix;
                matrix.countOut = vm -> getLength(2);
                if (matrix.countOut < 1) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                try {
                    for(int i = 1; i <= matrix.countOut; i++) {
                        vm -> getTableField(i, 2);
                        if (!vm -> isTable(-1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                        if (i == 1) {
                            matrix.countIn = vm -> getLength(-1);
                            if (matrix.countIn < 1) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                            int size = matrix.countIn*matrix.countOut;
                            matrix.matrix = new float[(size - 1)];
                        }
                        for(int j = 1; j <= matrix.countIn; j++) {
                            vm -> getTableField(i, 3);
                            if (!vm -> isNumber(-1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
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

        API::bind(vm, "sound", "set3D", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isBool(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                bool state = vm -> getBool(2);
                vm -> setBool(sound -> set3D(state));
                return 1;
            });
        });

        API::bind(vm, "sound", "set3DAttributes", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 3) || (!vm -> isUserData(1)) || (!vm -> isTable(2)) || (!vm -> isTable(3))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Math::Vector3D position, velocity;
                vm -> getTableField("x", 2);
                vm -> getTableField("y", 2);
                vm -> getTableField("z", 2);
                vm -> getTableField("x", 3);
                vm -> getTableField("y", 3);
                vm -> getTableField("z", 3);
                for(int i = -1; i >= -6; i--) {
                    if (!vm -> isNumber(i)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
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

        API::bind(vm, "sound", "set3DConeSettings", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isTable(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::ConeSetting3D setting;
                vm -> getTableField("insideAngle", 2);
                vm -> getTableField("outsideAngle", 2);
                vm -> getTableField("outsideVolume", 2);
                for(int i = -1; i >= -3; i--) {
                    if (!vm -> isNumber(i)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                }
                setting.insideAngle = vm -> getFloat(-3);
                setting.outsideAngle = vm -> getFloat(-2);
                setting.outsideVolume = vm -> getFloat(-1);
                vm -> setBool(sound -> set3DConeSettings(setting));
                return 1;
            });
        });

        API::bind(vm, "sound", "set3DConeOrientation", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isTable(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Math::Vector3D orientation;
                vm -> getTableField("x", 2);
                vm -> getTableField("y", 2);
                vm -> getTableField("z", 2);
                for(int i = -1; i >= -3; i--) {
                    if (!vm -> isNumber(i)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                }
                orientation.x = vm -> getFloat(-3);
                orientation.y = vm -> getFloat(-2);
                orientation.z = vm -> getFloat(-1);
                vm -> setBool(sound -> set3DConeOrientation(orientation));
                return 1;
            });
        });

        API::bind(vm, "sound", "set3DDistanceFilter", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 3) || (!vm -> isUserData(1)) || (!vm -> isBool(2)) || (!vm -> isTable(3))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::DistanceFilter3D filter;
                vm -> getTableField("customLevel", 3);
                vm -> getTableField("centerFrequency", 3);
                for(int i = -1; i >= -2; i--) {
                    if (!vm -> isNumber(i)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                }
                filter.isEnabled = vm -> getBool(2);
                filter.customLevel = vm -> getFloat(-2);
                filter.centerFrequency = vm -> getFloat(-1);
                vm -> setBool(sound -> set3DDistanceFilter(filter));
                return 1;
            });
        });

        API::bind(vm, "sound", "set3DDopplerLevel", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> set3DDopplerLevel(value));
                return 1;
            });
        });

        API::bind(vm, "sound", "set3DLevel", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> set3DLevel(value));
                return 1;
            });
        });

        API::bind(vm, "sound", "set3DRange", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 3) || (!vm -> isUserData(1)) || (!vm -> isNumber(2)) || (!vm -> isNumber(3))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::Range3D range;
                range.minDistance = vm -> getFloat(2);
                range.maxDistance = vm -> getFloat(3);
                vm -> setBool(sound -> set3DRange(range));
                return 1;
            });
        });

        API::bind(vm, "sound", "set3DOcclusion", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isTable(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::Occlusion3D occlusion;
                vm -> getTableField("directOcclusion", 2);
                vm -> getTableField("reverbOcclusion", 2);
                for(int i = -1; i >= -2; i--) {
                    if (!vm -> isNumber(i)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                }
                occlusion.directOcclusion = vm -> getFloat(-2);
                occlusion.reverbOcclusion = vm -> getFloat(-1);
                vm -> setBool(sound -> set3DOcclusion(occlusion));
                return 1;
            });
        });


        API::bind(vm, "sound", "set3DSpread", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isUserData(1)) || (!vm -> isNumber(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                float value = vm -> getFloat(2);
                vm -> setBool(sound -> set3DSpread(value));
                return 1;
            });
        });

        API::bind(vm, "sound", "play", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> play());
                return 1;
            });
        });
    
        API::bind(vm, "sound", "stop", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isUserData(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setBool(sound -> stop());
                return 1;
            });
        });

        API::bind(vm, "sound", "getPriority", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> getPriority());
                return 1;
            });
        });

        API::bind(vm, "sound", "getLoopCount", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> getLoopCount());
                return 1;
            });
        });

        API::bind(vm, "sound", "getLoopPoint", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::LoopPoint point;
                sound -> getLoopPoint(point);
                vm -> setNumber(static_cast<int>(point.start));
                vm -> setNumber(static_cast<int>(point.end));
                return 2;
            });
        });

        API::bind(vm, "sound", "getPitch", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> getPitch());
                return 1;
            });
        });

        API::bind(vm, "sound", "getFrequency", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> getFrequency());
                return 1;
            });
        });

        API::bind(vm, "sound", "getVolume", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> getVolume());
                return 1;
            });
        });

        API::bind(vm, "sound", "getAudibility", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> getAudibility());
                return 1;
            });
        });

        API::bind(vm, "sound", "getPosition", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(static_cast<int>(sound -> getPosition()));
                return 1;
            });
        });

        API::bind(vm, "sound", "getMixMatrix", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
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

        API::bind(vm, "sound", "get3DAttributes", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
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

        API::bind(vm, "sound", "get3DConeSettings", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
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

        API::bind(vm, "sound", "get3DConeOrientation", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
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

        API::bind(vm, "sound", "get3DDistanceFilter", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
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

        API::bind(vm, "sound", "get3DDopplerLevel", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> get3DDopplerLevel());
                return 1;
            });
        });
    
        API::bind(vm, "sound", "get3DLevel", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> get3DLevel());
                return 1;
            });
        });
    
        API::bind(vm, "sound", "get3DRange", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::Range3D range;
                sound -> get3DRange(range);
                vm -> setNumber(range.minDistance);
                vm -> setNumber(range.maxDistance);
                return 2;
            });
        });
    
        API::bind(vm, "sound", "get3DOcclusion", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                Vital::Type::Audio::Occlusion3D occlusion;
                sound -> get3DOcclusion(occlusion);
                vm -> createTable();
                vm -> registerNumber("directOcclusion", occlusion.directOcclusion);
                vm -> registerNumber("reverbOcclusion", occlusion.reverbOcclusion);
                return 1;
            });
        });

        API::bind(vm, "sound", "get3DSpread", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1)) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                auto sound = fetchSound(vm -> getUserData(1));
                vm -> setNumber(sound -> get3DSpread());
                return 1;
            });
        });
    }

    Vital::System::Audio::Sound::vsdk_sound* Audio::fetchSound(void* userdata) {
        auto sound = static_cast<Vital::System::Audio::Sound::vsdk_sound*>(userdata);
        if (!Vital::System::Audio::Sound::isInstance(sound)) throw std::runtime_error(ErrorCode["invalid-entities"]);
        return sound;
    }
}