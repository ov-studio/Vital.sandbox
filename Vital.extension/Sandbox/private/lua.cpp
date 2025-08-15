/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: private: lua.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Lua Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Sandbox/public/lua.h>
#include <Vital.sandbox/Sandbox/lua/public/api.h>
#include <Vital.sandbox/System/public/vital.h>
#include <Vital.sandbox/System/public/crypto.h>
#include <Vital.sandbox/System/public/rest.h>
#include <Vital.sandbox/Type/public/timer.h>


/////////////////////////////////
// Vital: Godot: Sandbox: Lua //
/////////////////////////////////

namespace Vital::Godot::Sandbox {
    Lua* instance = nullptr;
    Vital::Sandbox::Lua::create* vm = nullptr;

    Lua* Lua::fetch() {
        instance = instance ? instance : memnew(Lua());
        return instance;
    }

    Lua::Lua() {
        godot::UtilityFunctions::print("Initialized Lua vm");

        //auto serial = Vital::System::getSerial();
        //godot::UtilityFunctions::print(serial.c_str());

        vm = new Vital::Sandbox::Lua::create();

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

        //Ref<Environment> env = get_viewport()->get_environment();
        //auto stuff = Environment::get_ssao_intensity();
        //godot::UtilityFunctions::print(stuff);

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

    void Lua::process(double delta) {
        //godot::UtilityFunctions::print("rendered");
        
        // Lua script to run
        std::string rwString = R"(
            local a = 10
            local b = 20
            return a * b
        )";

        vm -> loadString(rwString);

        auto scene_tree = godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton()->get_main_loop());
        if (scene_tree) {
            auto root = scene_tree->get_root();
            if (root) {
                std::vector<godot::WorldEnvironment*> nodes;
                Vital::Godot::Engine::Singleton::find_nodes_by_type(root, nodes, 1);
                auto env_node = nodes.at(0);
                if (env_node) {
                    auto world_env = godot::Object::cast_to<godot::WorldEnvironment>(env_node);
                    if (world_env) {
                        auto environment = world_env->get_environment();
                        if (environment.is_valid()) {
                            // Access the SSAO properties
                            //bool ssao_enabled = environment->get_ssao_enabled();
                            float ssao_intensity = environment->get_ssao_intensity();
    
                            //godot::UtilityFunctions::print("SSAO Enabled: ", ssao_enabled ? "Yes" : "No");
                            godot::UtilityFunctions::print("SSAO Intensity: ", ssao_intensity);
                        } else {
                            godot::UtilityFunctions::print("Environment resource is null");
                        }
                    } else {
                        godot::UtilityFunctions::print("Node is not a WorldEnvironment");
                    }
                } else {
                    godot::UtilityFunctions::print("WorldEnvironment node not found");
                }
            }
        }
        
        /*
            auto *root_node = get_tree()->get_root()->get_node("Lua"); // Gets the root of the scene tree
            if (root_node) {
                godot::UtilityFunctions::print("yes")
            } else {
                godot::UtilityFunctions::print("Node not found");
            }
        */
    }
}