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
    std::cout << "Instantiated Platform: " << Vital::System::getPlatform() << std::endl;


    Vital::Sandbox::Lua::API::boot();
    Vital::Sandbox::Lua::API::onErrorHandle([](const std::string& err) -> void {
        std::cout << "\n" << err;
    });
    auto luaVM = new Vital::Sandbox::Lua::create();
    std::cout << "Instantiated VM: Lua" << std::endl;


    Vital::System::Event::bind("Network:@PeerConnect", [](Vital::Type::Stack::Instance arguments) -> void {
        std::cout << "\n[Client - " << arguments.getUnsignedLong("peerID") << "]: Connected";
        Vital::Type::Stack::Instance buffer;
        buffer.push("Network:message", "Hello From Server");
        Vital::System::Network::emit(buffer, arguments.getUnsignedLong("peerID"), false);
    });
    Vital::System::Event::bind("Network:@PeerMessage", [=](Vital::Type::Stack::Instance arguments) -> void {
        //if (arguments.isString("Network:message")) std::cout << "\n[Client - " << arguments.getUnsignedLong("peerID") << "]: " << arguments.getString("Network:message");
        if (arguments.isString("Network:name")) {
            std::cout << "\n Received Network:name - " << arguments.getString("Network:name");
            // TODO: Handle it better  to avoid injection/hacky solutions; store a ref in memory somehow....
            std::string rw = "network.execNetwork([[" + arguments.getString("Network:payload") + "]])";
            luaVM -> loadString(rw);
        }
    });
    Vital::System::Event::bind("Network:@PeerDisconnect", [](Vital::Type::Stack::Instance arguments) -> void {
        std::cout << "\n[Client - " << arguments.getUnsignedLong("peerID") << "]: Disconnected";
    });
    Vital::System::Network::start(Vital::Type::Network::Address {"127.0.0.1", 22003});
    std::cout << "Instantiated Network" << std::endl;


    std::string rwString = R"(
        timer:create(function()
            print("EXECUTED TIMER")
        end, 5000, 3)

        timer:create(function()
            print("EXECUTED TIMER")
        end, 1000, 3)
    )";
    luaVM -> loadString(rwString);

    do {
        Vital::System::Network::update();
    } while(true);
    return 1;
}
