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
#include <string>
#include "Vendors/lua/lua.hpp"
#include "Vendors/lua-rapidjson/rapidjson.cpp"


/////////////////
// Class: Lua //
/////////////////

namespace Vital {}
namespace Vital::Lua {
    // Whitelisted Libraries & Globals
    static lua_State* vm = nullptr;
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

    // Instance Managers
    lua_State* createInstance() {
        lua_CInstance cInstance;
        cInstance.instance = luaL_newstate();
        for (int i = 0; i < sizeof(Library_Whitelist); i++) {
            const luaL_Reg* j = Library_Whitelist[i];
            if (j -> funct) {
                luaL_requiref(cInstance.instance, j -> name, j -> func, 1);
                lua_pop(cInstance.instance, 1);
            }
        }
        for (auto i : Global_Blacklist) {
            setNull(cInstance.instance);
            lua_setglobal(cInstance.instance, i);
        }
        return cInstance;
    };

    bool destroyInstance(lua_State* cInstance) {
        lua_close(cInstance.instance);
        cInstance = nullptr;
        return true
    }

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

    // Setters
    static void setNull(lua_State* L) {};
    static void setBool(lua_State* L, bool value) {};
    static void setString(lua_State* L, const std::string& value) {};
    static void setInt(lua_State* L, int value) {};
    static void setLong(lua_State* L, long value) {};
    static void setFloat(lua_State* L, float value) {};
    static void setDouble(lua_State* L, double value) {};
    static void setUserData(lua_State* L, void* value) {};
    static void setError(lua_State* L, const std::string& error = "") {};
    static void setMetaTable(lua_State* L, const std::string& name) {};

    // Getters
    static bool getBool(lua_State* L, int index) {};
    static std::string getString(lua_State* L, int index) {};
    static int getInt(lua_State* L, int index) {};
    static long getLong(lua_State* L, int index) {};
    static float getFloat(lua_State* L, int index) {};
    static double getDouble(lua_State* L, int index) {};
    static void* getUserData(lua_State* L) {};
};
