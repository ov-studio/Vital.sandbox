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
#include <System/public/file.h>
#include <System/public/audio.h>
#include <System/public/event.h>
#include <System/public/network.h>
#include <Sandbox/lua/public/api.h>
#include <Sandbox/js/public/api.h>


///////////
// Root //
//////////

int main() {
    Vital::System::setPlatform("client");
    std::cout << "Launched Platform: " << Vital::System::getPlatform() << std::endl;
    std::cout << "Platform Serial: " << Vital::System::getSystemSerial() << std::endl;

    Vital::Type::Timer::Instance([](Vital::Type::Timer::Instance* self) -> void {
        std::cout << "\nC++ Timer executed!";
    }, 1000, 5);

    Vital::System::Event::bind("Network:@PeerMessage", [](Vital::Type::Stack::Instance arguments) -> void {
        std::cout << "\n[Server]: " << arguments.getString("message") << " " << arguments.getString("message2");
        Vital::Type::Stack::Instance buffer;
        buffer.push("message", "Hello From Client");
        Vital::System::Network::emit(buffer);
    });
    Vital::System::Event::bind("Network:@PeerDisconnect", [](Vital::Type::Stack::Instance arguments) -> void {
        std::cout << "\n[Server]: Disconnected";
    });
    Vital::System::Network::start(Vital::Type::Network::Address {"127.0.0.1", 22003});

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

        local rwBuffer = [[print('\nHurray, You ran me via `engine.loadString` successfully!')]]
        engine.loadString(rwBuffer)

        timer:create(function()
            print("EXECUTED TIMER")
        end, 5000, 3)
        print("Tail stack reached")


        local loadBuffer = [[
            print("I AM LOADED!")
        ]]
        local result = engine.loadString(loadBuffer, false)
        print(result)
        result()

        network:emit("mynetwork", true, false, "test1", "test2")
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
    std::string rootPath = "index.lua";
    if (!Vital::System::File::exists(rootPath)) printError("'" + rootPath + "' non-existent");
    else {
        Vital::Sandbox::Lua::API::boot();
        Vital::Sandbox::Lua::API::onErrorHandle(printError);
        auto vm = new Vital::Sandbox::Lua::create();
        vm -> loadString(Vital::System::File::read(rootPath));
    }
    */
    do {
        Vital::System::Network::update();
    } while (true);
    return 1;
}
