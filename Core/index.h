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
                    setNull(vm);
                    lua_setglobal(vm, i);
                }
            }

            bool destroy() {
                if (isUnloaded) return false;
                isUnloaded = true;
                lua_close(vm);
                vm = nullptr;
                return true
            }

            bool isNull(int index) {

            };

            bool isString(int index) {

            };

            bool isNumber(int index) {

            };

            int getArgCount(lua_State* L) {

            };

            void setNull(lua_State* L) {

            };

            void setBool(bool value) {

            };

            void setString(const std::string& value) {

            };

            void setInt(int value) {

            };

            void setLong(long value) {

            };

            void setFloat(float value) {

            };

            void setDouble(double value) {

            };

            void setUserData(void* value) {

            };

            void setError(const std::string& error = "") {

            };

            void setMetaTable(const std::string& name) {

            };

            // Getters
            bool getBool(int index) {

            };

            std::string getString(int index) {

            };

            int getInt(int index) {

            };

            long getLong(int index) {

            };

            float getFloat(int index) {

            };

            double getDouble(int index) {

            };

            void* getUserData() {

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
