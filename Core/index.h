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
    const char* Global_Blacklist[] = {"dofile", "load", "loadfile"};
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
    };

    class create {
        private:
            bool isUnloaded = false;
            lua_State* vm = nullptr;
        public:
            create() {
                vm = luaL_newstate();
                for (int i = 0; i < sizeof(Library_Whitelist); i++) {
                    const luaL_Reg* j = Library_Whitelist[i];
                    if (j -> funct) {
                        luaL_requiref(vm, j -> name, j -> func, 1);
                        lua_pop(vm, 1);
                    }
                }
                for (auto i : Global_Blacklist) {
                    setNil(vm);
                    lua_setglobal(vm, i);
                }
            }

            bool destroy() {
                if (isUnloaded) return false;
                isUnloaded = true;
                lua_close(vm);
                delete vm;
                vm = nullptr;
                return true
            }

            bool isNil(int index) return lua_isnoneornil(vm, index);
            bool isBool(int index) return lua_isboolean(vm, index);
            bool isString(int index) return lua_isstring(vm, index);
            bool isNumber(int index) return lua_isnumber(vm, index);
            bool isTable(int index) return lua_istable(vm, index);
            bool isThread(int index) return lua_isthread(vm, index);
            bool isUserData(int index) return lua_isuserdata(vm, index);
            bool isFunction(int index) return lua_isfunction(vm, index);

            void setGlobal(std::string& index) return lua_setglobal(vm, index.c_str());
            void setNil() return lua_pushnil(vm);
            void setBool(bool value) return lua_pushboolean(vm, static_cast<int>(value))
            void setString(std::string& value) return lua_pushstring(vm, value.c_str());
            void setInt(int value) return lua_pushnumber(vm, (lua_Number)value);
            void setFloat(float value) return lua_pushnumber(vm, (lua_Number)value);
            void setLong(long value) return lua_pushnumber(vm, (lua_Number)value);
            void setDouble(double value) return lua_pushnumber(vm, (lua_Number)value);
            void setTable(int index = 1) return lua_settable(vm, int index);
            void createMetaTable(std::string& value) return luaL_newmetatable(vm, value.c_str());
            void setMetaTable(int index = 1) return lua_setmetatable(vm, int index);
            void setUserData(void* value) return lua_pushlightuserdata(vm, value);

            int getArgCount() return lua_gettop(vm);
            bool getGlobal(std::string& index) return lua_getglobal(vm, index);
            bool getBool(int index = 1) return static_cast<bool>(lua_toboolean(vm, index));
            std::string getString(int index = 1) return lua_tostring(vm, index) || std::string("");
            int getInt(int index = 1) return lua_tonumber(vm, index);
            float getFloat(int index = 1) return lua_tonumber(vm, index);
            double getDouble(int index = 1) return lua_tonumber(vm, index);
            long getLong(int index = 1) return lua_tonumber(vm, index);
            bool getTable(int index = 1) return lua_gettable(vm, index);
            bool getMetaTable(int index = 1) return lua_getmetatable(vm, index);
            void* getUserData(int index = 1) return lua_touserdata(vm, index);

            // TODO: WIP
            bool setError(std::string& error = "") {
                
            };
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
};
