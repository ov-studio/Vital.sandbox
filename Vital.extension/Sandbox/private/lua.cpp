#include <Sandbox/public/lua.h>
#include <../Vital.sandbox/Sandbox/lua/public/api.h>
#include <../Vital.sandbox/System/public/vital.h>
#include <../Vital.sandbox/System/public/crypto.h>
#include <../Vital.sandbox/System/public/rest.h>
#include <../Vital.sandbox/Type/public/timer.h>

#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/environment.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/window.hpp>

Vital::Sandbox::Lua::create* luaVM = nullptr;

ExampleLUA* ExampleLUA::singleton_instance = nullptr;

ExampleLUA* ExampleLUA::fetch() {
    godot::UtilityFunctions::print("called");
    if (!singleton_instance) {
        godot::UtilityFunctions::print("intializing");
        singleton_instance = memnew(ExampleLUA());
    }
    return singleton_instance;
}

ExampleLUA::ExampleLUA() {
	godot::UtilityFunctions::print("Initialized Lua vm");

    //auto serial = Vital::System::getSerial();
	//godot::UtilityFunctions::print(serial.c_str());

    luaVM = new Vital::Sandbox::Lua::create();

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
	luaVM -> loadString(rwString);
	double result = luaVM -> getInt(-1);
	godot::UtilityFunctions::print(result);
    */

    std::string rwString = R"(
        thread:create(function(self)
            print("Initiated GET request")
            print(rest.get("https://jsonplaceholder.typicode.com/posts/1"))
            print("Processed GET rest")
        end):resume()
    )";
	luaVM -> loadString(rwString);

    //Ref<Environment> environment = viewport->get_environment();
    //if (environment.is_valid()) {
        // You can now call methods on environment
    //}


    //auto env = get_viewport()->get_environment();


    //Environment env;
    //auto stuff = env.get_ssao_intensity();
    //godot::UtilityFunctions::print(stuff);

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
	//ClassDB::register_abstract_class<ExampleLUA>();

}

void ExampleLUA::process(double delta) {
	//godot::UtilityFunctions::print("rendered");
	
	// Lua script to run
	std::string rwString = R"(
        local a = 10
        local b = 20
        return a * b
    )";

	luaVM -> loadString(rwString);

    auto scene_tree = godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton()->get_main_loop());
    if (scene_tree) {
        auto root = scene_tree->get_root();
        if (root) {
            // Replace "Path/To/Your/Node3D" with the actual path in your scene tree
            auto my_node3d = root->find_child("test", true, false);

            if (my_node3d) {
                // Successfully retrieved the Node3D, you can now use it
                auto node_path = my_node3d->get_path();
                godot::UtilityFunctions::print("Node3D found!", node_path);
            } else {
                godot::UtilityFunctions::print("Node3D not found at specified path.");
            }
        }
    }
    
    /*
        auto *root_node = get_tree()->get_root()->get_node("ExampleLUA"); // Gets the root of the scene tree
        if (root_node) {
            godot::UtilityFunctions::print("yes")
        } else {
            godot::UtilityFunctions::print("Node not found");
        }
    */
}
