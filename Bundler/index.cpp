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

void outputConsole(const std::string& message) { std::cout << "\nVital.bundler | " << message; }
void genPackage(const std::string& name, const std::string& entry, std::vector<std::string> modules) {
    std::string rwBundle = "namespace Vital::Sandbox::" + name + " {\nstd::vector<std::string> rwBundle = {";
    outputConsole("Packaging " + name + "...");
    for (auto i : modules) {
        std::string path = entry + i;
        if (!Vital::System::File::exists(path)) {
            outputConsole("Invalid File: " + path);
            throw 0;
        }
        else {
            rwBundle += "\nR\"" + Vital::Signature + "(\n" + Vital::System::File::read(path) + "\n)" + Vital::Signature + "\",";
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
#include <System/public/thread.h>

#include <iostream>
#include <thread>

void my_func() {
    std::cout << "PAUSED FOR 3 SECONDS";
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    for (int j = 0; j < 3; ++j)
    {
        std::cout << "\n " << j;
    }
}

void test() {
    //Vital::System::Thread::create st1(std::thread(my_func));
    auto sthread = Vital::System::Thread::create(my_func);
    std::cout << "TAIL STACK";
}

int main() {
    genPackage("Lua", "Sandbox/lua/module/", Vital::Sandbox::Lua::module);
    genPackage("JS", "Sandbox/js/module/", Vital::Sandbox::JS::module);
    Vital::System::Audio::start();
    Vital::Sandbox::Lua::API::boot();
    Vital::Sandbox::Lua::API::onErrorHandle([](const std::string& err) -> void {
        std::cout << "\n" << err;
    });
    std::string rwString = R"(
        print("\n")
        local buffer = "test"
        --print(crypto.decode(buffer))
        --local testSound = sound.create("C:/Users/Tron/Documents/GITs/Test/Bells.mp3")
        --sound.play(testSound)
        --sound.setPitch(testSound, 5)
        --sound.setLooped(testSound, true)
        --print("IS 3D: "..tostring(sound.is3D(testSound)))
        --table.print(sound.getMixMatrix(testSound))

        --coroutine.resume(coroutine.create(function()
            --print("EXECUTED THREAD 1 & PAUSED FOR 10s")
            --engine.sleep(10000)
            --print("RESUMED THREAD 1")
            --local testSound = sound.create("C:/Users/Tron/Documents/GITs/Test/Bells.mp3")
            --sound.play(testSound)
        --end))

        --coroutine.resume(coroutine.create(function()
            --print("EXECUTED THREAD 2 & PAUSED FOR 3s")
            --coroutine.sleep(3000)
            --print("RESUMED THREAD 2")
        --end))
    )";
    auto testVM = new Vital::Sandbox::Lua::create();
    testVM -> loadString(rwString);

    test();

    /*
    Vital::Sandbox::JS::API::boot();
    Vital::Sandbox::JS::API::onErrorHandle([](const std::string& err) -> void {
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
    */

    // TODO: REMOVE LATER
    do {
        Vital::System::Audio::update();
    } while (true);
    return 1;
}
