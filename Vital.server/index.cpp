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
    Vital::System::setSystemPlatform("server");
    std::cout << "Instantiated Platform: " << Vital::System::getSystemPlatform() << std::endl;

    Vital::Sandbox::Lua::API::boot();
    Vital::Sandbox::Lua::API::createErrorHandle([](const std::string& err) -> void {
        std::cout << "\n" << err;
    });
    auto luaVM = new Vital::Sandbox::Lua::create();
    std::cout << "Instantiated VM: Lua" << std::endl;


    Vital::System::Event::bind("Network:@PeerConnect", [](Vital::Type::Stack arguments) -> void {
        std::cout << "\n[Client - " << arguments.get<unsigned long>("peerID") << "]: Connected";
        Vital::Type::Stack buffer;
        buffer.push("Network:message", "Hello From Server");
        Vital::System::Network::emit(buffer, arguments.get<unsigned long>("peerID"), false);
    });
    Vital::System::Event::bind("Network:@PeerMessage", [=](Vital::Type::Stack arguments) -> void {
        //if (arguments.isString("Network:message")) std::cout << "\n[Client - " << arguments.getUnsignedLong("peerID") << "]: " << arguments.getString("Network:message");
        if (arguments.isString("Network:name")) {
            std::cout << "\n Received Network:name - " << arguments.get<std::string>("Network:name");
            // TODO: Handle it better  to avoid injection/hacky solutions; store a ref in memory somehow....
            std::string rw = "network.execNetwork([[" + arguments.get<std::string>("Network:payload") + "]])";
            luaVM -> loadString(rw);
        }
    });
    Vital::System::Event::bind("Network:@PeerDisconnect", [](Vital::Type::Stack arguments) -> void {
        std::cout << "\n[Client - " << arguments.get<unsigned long>("peerID") << "]: Disconnected";
    });
    Vital::System::Network::start(Vital::Type::Network::Address {"127.0.0.1", 22003});
    std::cout << "Instantiated Network" << std::endl;


    std::string rwString = R"(
        timer:create(function()
            print("EXECUTED TIMER")
        end, 5000, 3)
    )";
    luaVM -> loadString(rwString);

    do {
        Vital::System::Network::update();
    } while(true);
    return 1;
}
