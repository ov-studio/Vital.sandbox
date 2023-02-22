/*----------------------------------------------------------------
     Resource: Vital.client
     Script: index.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Bundler Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/network.h>
#include <System/public/audio.h>
#include <Sandbox/lua/public/api.h>
#include <Sandbox/js/public/api.h>


///////////
// Root //
//////////



int main() {
    Vital::System::setPlatform("client");
    std::cout << "\nLaunched Platform: " << Vital::System::getPlatform();
    std::cout << "\nPlatform Serial: " << Vital::System::getSystemSerial();
    Vital::System::Network::start(Vital::Type::Network::Address{"127.0.0.1", 22003});
    Vital::System::Audio::start();
    Vital::Sandbox::Lua::API::boot();
    Vital::Sandbox::Lua::API::onErrorHandle([](const std::string& err) -> void {
        std::cout << "\n" << err;
    });

    std::string rwString = R"(
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

    do {
        Vital::System::Network::update();
        Vital::System::Audio::update();
    } while (true);
    return 1;
}
