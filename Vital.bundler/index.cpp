/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Bundler: index.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
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

#define vSDK_Client true

int main() {
    genPackage("Lua", "../Vital.sandbox/Sandbox/lua/module/", Vital::Sandbox::Lua::module);
    genPackage("JS", "../Vital.sandbox/Sandbox/js/module/", Vital::Sandbox::JS::module);
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

        timer:create(function()
            print("EXECUTED TIMER")
        end, 5000, 3)
        print("Tail stack reached")
    )";
    //std::cout<<"\n Main Thread : " << std::this_thread::get_id();
    auto testVM = new Vital::Sandbox::Lua::create();
    testVM -> loadString(rwString);

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

    std::wcout << "\nSerial: " << Vital::System::getSystemSerial() << "\n";

    // TODO: REMOVE LATER
    do {
        Vital::System::Audio::update();
    } while (true);
    return 1;
}
