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
	//void AddInt(lua_State* L, const std::string& name, int data) {};

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
	static void setNull(lua_State* L);
	static void setBool(lua_State* L, bool data) {};
	static void setString(lua_State* L, const std::string& data) {};
	static void setInt(lua_State* L, int data) {};
	static void setLong(lua_State* L, long data) {};
	static void setFloat(lua_State* L, float data) {};
	static void setFloat2(lua_State* L, const XMFLOAT2& data) {};
	static void setFloat3(lua_State* L, const XMFLOAT3& data) {};
	static void setFloat4(lua_State* L, const XMFLOAT4& data) {};
	static void setDouble(lua_State* L, double data) {};
	static void setUserData(lua_State* L, void* data) {};
	static void setError(lua_State* L, const std::string& error = "");
	static void setMetaTable(lua_State* L, const std::string& name);
};
