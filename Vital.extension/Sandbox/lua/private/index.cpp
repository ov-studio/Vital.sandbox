/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: private: lua.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/public/index.h>
#include <Vital.sandbox/Sandbox/lua/public/api.h>
#include <Vital.sandbox/System/public/vital.h>
#include <Vital.sandbox/System/public/crypto.h>
#include <Vital.sandbox/System/public/rest.h>
#include <Vital.sandbox/Type/public/timer.h>

#include <Vital.extension/Sandbox/lua/api/public/ssr.h>
#include <Vital.extension/Sandbox/lua/api/public/ssao.h>


/////////////////////////////////
// Vital: Godot: Sandbox: Lua //
/////////////////////////////////

namespace Vital::Godot::Sandbox::Lua {
    Singleton* instance = nullptr;
    Vital::Sandbox::Lua::create* vm = nullptr;

    Singleton* Singleton::fetch() {
        instance = instance ? instance : memnew(Singleton());
        return instance;
    }

    Singleton::Singleton() {
        godot::UtilityFunctions::print("Initialized Lua vm");

        //auto serial = Vital::System::getSerial();
        //godot::UtilityFunctions::print(serial.c_str());

        vm = new Vital::Sandbox::Lua::create({
            {API::SSR::bind, API::SSR::inject},
            {API::SSAO::bind, API::SSAO::inject},
        });

        /*
        std::string rwString = R"(
            local a = 10
            local b = 20
            print("Can you see this", "again")
            print("SHA256 of Hello", crypto.hash("SHA256", "hello"))
            timer:create(function()
                print("Executing timer")
            end, 2500, 5)
            return a + b
        )";
        vm -> loadString(rwString);
        double result = vm -> getInt(-1);
        godot::UtilityFunctions::print(result);
        */

        std::string rwString = R"(
            thread:create(function(self)
                print("Initiated GET request")
                print(rest.get("https://jsonplaceholder.typicode.com/posts/1"))
                print("Processed GET rest")
            end):resume()
        )";
        vm -> loadString(rwString);

        /*
        Vital::Type::Timer([](auto self) -> void {
            godot::UtilityFunctions::print("executed c++ timer");
        }, 5000, 0);
        */

        // Run in another thread
        /*
        Vital::Type::Thread([=](Vital::Type::Thread* thread) -> void {
            try {
                std::string url = "https://jsonplaceholder.typicode.com/posts/1";
                std::string response = Vital::System::REST::get(url);
                godot::UtilityFunctions::print(response.c_str());
            }
            catch(const std::runtime_error& error) { godot::UtilityFunctions::print(error.what()); }
        }).detach();
        */
        
        //auto stuff = Vital::System::Crypto::hash("SHA256", "hello");
        //godot::UtilityFunctions::print(stuff.c_str());
        //ClassDB::register_abstract_class<Lua>();

    }

    void Singleton::process(double delta) {
        //godot::UtilityFunctions::print("rendered");
        
        // Lua script to run
        std::string rwString = R"(
            print("SSAO Enabled: ", ssao.setEnabled(true))
            print("SSAO Intensity: ", ssao.getIntensity())
        )";
        vm -> loadString(rwString);
    }
}