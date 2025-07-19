#include "example_class.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <../Vital.sandbox/Vendor/lua/lua.hpp>

#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <../Vital.sandbox/System/public/vital.h>
#include <../Vital.sandbox/System/public/crypto.h>

lua_State *L = nullptr;

ExampleClass::ExampleClass() {
	UtilityFunctions::print("works?");


	UtilityFunctions::print("init");

    Vital::System::setSystemPlatform("client");
    auto serial = Vital::System::getSystemSerial();
	UtilityFunctions::print(serial.c_str());


	L = luaL_newstate();

	// Open standard libraries
	luaL_openlibs(L);

	// Lua script to run
	const char *lua_script = R"(
        local a = 10
        local b = 20
        return a + b
    )";

	// Run the script
	if (luaL_loadstring(L, lua_script) || lua_pcall(L, 0, 1, 0)) {
	}

	// The result is on top of the stack
	if (lua_isnumber(L, -1)) {
		double result = lua_tonumber(L, -1);
		std::cout << "Result: " << result << std::endl;
		UtilityFunctions::print(result);
	} else {
		UtilityFunctions::print("unexpected result");
	}

	auto stuff = Vital::System::Crypto::hash("SHA256", "hello");
	
	UtilityFunctions::print(stuff.c_str());

	//ClassDB::register_abstract_class<ExampleClass>();

}

void ExampleClass::_bind_methods() {
	godot::ClassDB::bind_method(D_METHOD("print_type", "variant"), &ExampleClass::print_type);
}

void ExampleClass::print_type(const Variant &p_variant) const {
	print_line(vformat("Type: %d", p_variant.get_type()));
}

using namespace godot;

void ExampleClass::_process(double delta) {
	//UtilityFunctions::print("rendered");
	
	// Lua script to run
	const char *lua_script = R"(
        local a = 10
        local b = 20
        return a * b
    )";

	// Run the script
	if (luaL_loadstring(L, lua_script) || lua_pcall(L, 0, 1, 0)) {
	}

	// The result is on top of the stack
	if (lua_isnumber(L, -1)) {
		double result = lua_tonumber(L, -1);
		//UtilityFunctions::print(result);
	}


    /*
        auto *root_node = get_tree()->get_root()->get_node("ExampleClass"); // Gets the root of the scene tree
        if (root_node) {
            UtilityFunctions::print("yes")
        } else {
            UtilityFunctions::print("Node not found");
        }
    */
}
