/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Bundler: index.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Bundler Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/file.h>
#include <System/public/crypto.h>
#include <Sandbox/lua/public/index.h>
#include <Sandbox/js/public/index.h>


//////////////
// Bundler //
//////////////

void outputConsole(std::string message) { std::cout << "\nVital.bundler | " << message; }
void genPackage(std::string name, std::string entry, std::vector<std::string> modules) {
    std::string rwBundle = "namespace Vital::" + name + " {\nstd::vector<std::string> vBundle = {";
    outputConsole("Packaging " + name + "...");
    for (auto i : modules) {
        std::string path = entry + i;
        if (!Vital::System::File::exists(path)) {
            outputConsole("Invalid File: " + path);
            throw 0;
        }
        else {
            rwBundle += "\nR\"" + Vital::vSignature + "(\n" + Vital::System::File::read(path) + "\n)" + Vital::vSignature + "\",";
            outputConsole("Bundled File: " + path);
        }
    }
    rwBundle += "\n};\n}";
    std::string path = entry + "bundle.h";
    Vital::System::File::write(path, rwBundle);
    outputConsole("Packaged " + name + " successfully!");
}

#include <Sandbox/lua/public/api.h>
#include <Sandbox/js/public/api.h>
#include <System/public/audio.h>
#include <thread>

bool succeededOrWarn(const std::string& message, FMOD_RESULT result)
{
    if (result != FMOD_OK) {
        std::cerr << message << ": " << result << " " << FMOD_ErrorString(result) << std::endl;
        return false;
    }
    return true;
}

FMOD_RESULT F_CALLBACK channelGroupCallback(FMOD_CHANNELCONTROL* channelControl,
    FMOD_CHANNELCONTROL_TYPE controlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType,
    void* commandData1, void* commandData2)
{
    std::cout << "Callback called for " << controlType << std::endl;
    return FMOD_OK;
}

int main() {
    genPackage("Lua", "Sandbox/lua/module/", Vital::Sandbox::Lua::vModules);
    genPackage("JS", "Sandbox/js/module/", Vital::Sandbox::JS::vModules);
    /*
    Vital::Sandbox::Lua::API::boot();
    Vital::Sandbox::Lua::API::onErrorHandle([](std::string& err) -> void {
        std::cout << "\n" << err;
    });
    std::string rwString = R"(
        print("\n")
        local buffer = "test"
        print(crypto:decode(buffer))
    )";
    auto testVM = new Vital::Sandbox::Lua::create();
    testVM -> loadString(rwString);
    */

    Vital::Sandbox::JS::API::boot();
    Vital::Sandbox::JS::API::onErrorHandle([](std::string& err) -> void {
        std::cout << "\n" << err;
    });
    std::string rwString = R"(
    test = function() {
        return Duktape;
    }
    test()
    )";
    auto testVM = new Vital::Sandbox::JS::create();
    testVM -> loadString(rwString);
    std::string result = testVM -> getString(-1);
    std::cout << "\n" << "RESULT: " << result;



    FMOD_RESULT result2;

    FMOD::System* system = nullptr;
    // Create the main system object.
    result2 = FMOD::System_Create(&system);
    if (!succeededOrWarn("FMOD: Failed to create system object", result2))
        return 1;

    // Initialize FMOD.
    result2 = system->init(512, FMOD_INIT_NORMAL, nullptr);
    if (!succeededOrWarn("FMOD: Failed to initialise system object", result2))
        return 1;

    // Create the channel group.
    FMOD::ChannelGroup* channelGroup = nullptr;
    result2 = system->createChannelGroup("inGameSoundEffects", &channelGroup);
    if (!succeededOrWarn("FMOD: Failed to create in-game sound effects channel group", result2))
        return 1;

    // Create the sound.
    FMOD::Sound* sound = nullptr;
    system->createSound("C:/Users/Tron/Documents/GITs/Test/Bells.mp3", FMOD_DEFAULT, nullptr, &sound);

    // Play the sound.
    FMOD::Channel* channel = nullptr;
    result2 = system->playSound(sound, nullptr, false, &channel);
    if (!succeededOrWarn("FMOD: Failed to play sound", result2))
        return 1;

    // Assign the channel to the group.
    result2 = channel->setChannelGroup(channelGroup);
    if (!succeededOrWarn("FMOD: Failed to set channel group on", result2))
        return 1;

    // Set a callback on the channel.
    channel->setCallback(&channelGroupCallback);
    if (!succeededOrWarn("FMOD: Failed to set callback for sound", result2))
        return 1;

    bool isPlaying = false;
    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        channel->isPlaying(&isPlaying);

        system->update();
    } while (isPlaying);


    // Clean up.
    sound->release();
    channelGroup->release();
    system->release();
    return 1;
}
