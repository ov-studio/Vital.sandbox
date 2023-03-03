/*----------------------------------------------------------------
     Resource: Vital.server
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
#include <System/public/event.h>
#include <System/public/network.h>
#include <Sandbox/lua/public/api.h>
#include <Sandbox/js/public/api.h>


///////////
// Root //
//////////



int main() {
    Vital::System::setPlatform("server");
    std::cout << "\nLaunched Platform: " << Vital::System::getPlatform();

    Vital::System::Event::bind("Network:@PeerConnection", [](Vital::Type::Stack::Instance arguments) -> void {
        std::cout << "\n[Client - " << arguments.getUnsignedLong("peerID") << "]: Connected";
        Vital::Type::Stack::Instance buffer;
        buffer.push("message", "Hello From Server");
        buffer.push("message2", "Wavin'");
        Vital::System::Network::emit(buffer, arguments.getUnsignedLong("peerID"), false);
    });
    Vital::System::Event::bind("Network:@PeerMessage", [](Vital::Type::Stack::Instance arguments) -> void {
        std::cout << "\n[Client - " << arguments.getUnsignedLong("peerID") << "]: " << arguments.getString("message");
    });
    Vital::System::Event::bind("Network:@PeerDisconnection", [](Vital::Type::Stack::Instance arguments) -> void {
        std::cout << "\n[Client - " << arguments.getUnsignedLong("peerID") << "]: Disconnected";
    });
    Vital::System::Network::start(Vital::Type::Network::Address{"127.0.0.1", 22003});

    Vital::Sandbox::Lua::API::boot();
    Vital::Sandbox::Lua::API::onErrorHandle([](const std::string& err) -> void {
        std::cout << "\n" << err;
    });

    std::string rwString = R"(
        timer:create(function()
            print("EXECUTED TIMER")
        end, 5000, 3)
        print("Tail stack reached")
    )";
    auto testVM = new Vital::Sandbox::Lua::create();
    testVM -> loadString(rwString);

    do {
        Vital::System::Network::update();
    } while(true);
    return 1;
}
