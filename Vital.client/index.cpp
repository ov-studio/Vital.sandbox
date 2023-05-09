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
#include <Sandbox/lua/public/api.h>


///////////
// Root //
//////////

int main() {
    Vital::System::setPlatform("client");
    std::cout << "\nLaunched Platform: " << Vital::System::getPlatform();
    std::cout << "\nPlatform Serial: " << Vital::System::getSystemSerial();

    Vital::Type::Timer::Instance([](Vital::Type::Timer::Instance* self) -> void {
        std::cout << "\nC++ Timer executed!";
    }, 1000, 5);

    Vital::System::Event::bind("Network:@PeerMessage", [](Vital::Type::Stack::Instance arguments) -> void {
        std::cout << "\n[Server]: " << arguments.getString("message") << " " << arguments.getString("message2");
        Vital::Type::Stack::Instance buffer;
        buffer.push("message", "Hello From Client");
        Vital::System::Network::emit(buffer);
    });
    Vital::System::Event::bind("Network:@PeerDisconnection", [](Vital::Type::Stack::Instance arguments) -> void {
        std::cout << "\n[Server]: Disconnected";
    });
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

    } while (true);
    return 1;
}
