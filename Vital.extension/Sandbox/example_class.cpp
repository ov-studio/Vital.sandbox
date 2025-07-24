#include "example_class.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>

#include <../Vital.sandbox/Sandbox/lua/public/api.h>
#include <../Vital.sandbox/System/public/vital.h>
#include <../Vital.sandbox/System/public/crypto.h>
#include <../Vital.sandbox/System/public/rest.h>


Vital::Sandbox::Lua::create* luaVM = nullptr;


ExampleClass::ExampleClass() {
	UtilityFunctions::print("works?");


	UtilityFunctions::print("init");

    auto serial = Vital::System::getSerial();
	UtilityFunctions::print(serial.c_str());

    luaVM = new Vital::Sandbox::Lua::create();

	std::string rwString = R"(
        local a = 10
        local b = 20
        print("Can you see this", "again")
        print("SHA256 of Hello", crypto.hash("SHA256", "hello"))
        return a + b
    )";

	luaVM -> loadString(rwString);
	double result = luaVM -> getInt(-1);
	UtilityFunctions::print(result);

    try {
        std::string url = "https://jsonplaceholder.typicode.com/posts/1";
        std::string response = Vital::System::REST::get(url);
    	UtilityFunctions::print(response.c_str());
    }
    catch(const std::runtime_error& error) { UtilityFunctions::print(error.what()); }


	//auto stuff = Vital::System::Crypto::hash("SHA256", "hello");
	//UtilityFunctions::print(stuff.c_str());
	//ClassDB::register_abstract_class<ExampleClass>();

}

void ExampleClass::_bind_methods() {

}

void ExampleClass::_process(double delta) {
	//UtilityFunctions::print("rendered");
	
	// Lua script to run
	std::string rwString = R"(
        local a = 10
        local b = 20
        return a * b
    )";

	luaVM -> loadString(rwString);

    /*
        auto *root_node = get_tree()->get_root()->get_node("ExampleClass"); // Gets the root of the scene tree
        if (root_node) {
            UtilityFunctions::print("yes")
        } else {
            UtilityFunctions::print("Node not found");
        }
    */
}
