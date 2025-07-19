#include "example_class.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <../Vital.sandbox/Vendor/lua/lua.hpp>

#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include <iostream>
#include <algorithm>
#include <chrono>
#include <functional>
#include <filesystem>
#include <fstream>
#include <thread>
#include <exception>
#include <vector>
#include <map>
#include <variant>
#include <utility>
#include <string>
#include <sstream>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>

    std::string encode(const std::string& buffer) {
        BIO* bio = BIO_new(BIO_f_base64());
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO* bmem = BIO_new(BIO_s_mem());
        bio = BIO_push(bio, bmem);
        BIO_write(bio, buffer.c_str(), buffer.size());
        BIO_flush(bio);
        BUF_MEM* bptr;
        BIO_get_mem_ptr(bio, &bptr);
        std::string result(bptr->data, bptr->length);
        BIO_free_all(bio);
        return result;
    }
    
    std::string decode(const std::string& buffer) {
        BIO* bio = BIO_new(BIO_f_base64());
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO* bmem = BIO_new_mem_buf(buffer.c_str(), buffer.size());
        bio = BIO_push(bio, bmem);
        std::string result(buffer.size(), '\0');
        int decodedLength = BIO_read(bio, &result[0], buffer.size());
        BIO_free_all(bio);
        result.resize(decodedLength);
        return result;
    }
	
	std::pair<std::function<unsigned char* (const unsigned char*, size_t, unsigned char*)>, int> HashMode(const std::string& mode) {
        if (mode == "SHA1") return {::SHA1, SHA_DIGEST_LENGTH};
        else if (mode == "SHA224") return {::SHA224, SHA224_DIGEST_LENGTH};
        else if (mode == "SHA256") return {::SHA256, SHA256_DIGEST_LENGTH};
        else if (mode == "SHA384") return {::SHA384, SHA384_DIGEST_LENGTH};
        else if (mode == "SHA512") return {::SHA512, SHA512_DIGEST_LENGTH};
        else throw std::runtime_error("idk");
    }

	std::string hash(const std::string& mode, const std::string& buffer) {
        try {
            auto algorithm = HashMode(mode);
            int outputSize = algorithm.second;
            std::vector<unsigned char> output(outputSize);
            algorithm.first(reinterpret_cast<const unsigned char*>(buffer.data()), buffer.size(), output.data());
            std::stringstream ss;
            for (const auto& byte : output) {
                ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
            }
            return ss.str();
        }
        catch(...) { std::rethrow_exception(std::current_exception()); }
    }


lua_State *L = nullptr;

ExampleClass::ExampleClass() {
	UtilityFunctions::print("works?");


	UtilityFunctions::print("init");

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

	auto stuff = hash("SHA256", "hello");
	
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
