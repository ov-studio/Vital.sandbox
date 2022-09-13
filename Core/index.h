/*----------------------------------------------------------------
     Resource: Vital.lua
     Script: Core: index.h
     Author: vStudio
     Developer(s): Aviril, Mario, Tron
     DOC: 14/09/2022
     Desc: Module Initializer
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once


/////////////////
// Class: Lua //
/////////////////

namespace Vital::Lua {
    // Whitelisted Libraries & Globals
    static const luaL_Reg Library_Whitelist[] = {
        {"_G", luaopen_base},
        {LUA_TABLIBNAME, luaopen_table},
        {LUA_STRLIBNAME, luaopen_string},
        {LUA_MATHLIBNAME, luaopen_math},
        {LUA_DBLIBNAME, luaopen_debug},
        {LUA_COLIBNAME, luaopen_coroutine},
        {LUA_UTF8LIBNAME, luaopen_utf8},
        {"json", luaopen_rapidjson},
        {NULL, NULL}
    }, const char* Global_Blacklist[] = {
        "dofile", "load", "loadfile"
    };

	//register function to use in scripts
	//bool RegisterFunc(lua_State* L, const std::string& name, lua_CFunction function) {};
	// Registers Namespace
	//void RegisterNamespace(lua_State* L, const char* namespac, const wi::vector<std::string>& namespaceIndex) {};
	// Registers Library
	//void RegisterLibrary(lua_State* L, const std::string& tableName, const luaL_Reg* functions, const char* namespac = NULL, const wi::vector<std::string>& namespaceIndex = {}) {};
	// Registers Object
	//bool RegisterObject(lua_State* L, const std::string& tableName, void* object, const char* namespac = NULL, const wi::vector<std::string>& namespaceIndex = {}) {};
	// Add function to the previously registered object
	//void AddFunc(lua_State* L, const std::string& name, lua_CFunction function) {};
	// Add function array to the previously registered object
	//void AddFuncs(lua_State* L, const luaL_Reg* functions) {};
	// Add int member to registered object
	//void AddInt(lua_State* L, const std::string& name, int value) {};

	// Checkers
	static bool isNull(lua_State* L, int index) {};
	static bool isString(lua_State* L, int index) {};
	static bool isNumber(lua_State* L, int index) {};
	static int getArgCount(lua_State* L) {};

	// Getters
	static bool getBool(lua_State* L, int index) {};
	static std::string getString(lua_State* L, int index) {};
	static int getInt(lua_State* L, int index) {};
	static long getLong(lua_State* L, int index) {};
	static float getFloat(lua_State* L, int index) {};
	static XMFLOAT2 getFloat2(lua_State* L, int index) {};
	static XMFLOAT3 getFloat3(lua_State* L, int index) {};
	static XMFLOAT4 getFloat4(lua_State* L, int index) {};
	static double getDouble(lua_State* L, int index) {};
	static void* getUserData(lua_State* L) {};
	
	// Setters
	static void setNull(lua_State* L) {};
	static void setBool(lua_State* L, bool value) {};
	static void setString(lua_State* L, const std::string& value) {};
	static void setInt(lua_State* L, int value) {};
	static void setLong(lua_State* L, long value) {};
	static void setFloat(lua_State* L, float value) {};
	static void setFloat2(lua_State* L, const XMFLOAT2& value) {};
	static void setFloat3(lua_State* L, const XMFLOAT3& value) {};
	static void setFloat4(lua_State* L, const XMFLOAT4& value) {};
	static void setDouble(lua_State* L, double value) {};
	static void setUserData(lua_State* L, void* value) {};
	static void setError(lua_State* L, const std::string& error = "") {};
	static void setMetaTable(lua_State* L, const std::string& name) {};
};
