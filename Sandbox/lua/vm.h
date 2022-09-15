/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: vm.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: VM Handler
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include "Sandbox/lua/index.h"


/////////////////////
// Namespace: Lua //
/////////////////////

namespace Vital::Lua {
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
                for (auto& i : Global_Blacklist) {
                    setNil();
                    setGlobal(i);
                }
                for (auto& i : Global_Modules) {
                    loadString(Vital::FileSystem::read(std::filesystem::current_path() + "/modules/" + i));
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

            // Checkers //
            bool isNil(int index) return lua_isnoneornil(vm, index);
            bool isBool(int index) return lua_isboolean(vm, index);
            bool isString(int index) return lua_isstring(vm, index);
            bool isNumber(int index) return lua_isnumber(vm, index);
            bool isTable(int index) return lua_istable(vm, index);
            bool isThread(int index) return lua_isthread(vm, index);
            bool isUserData(int index) return lua_isuserdata(vm, index);
            bool isFunction(int index) return lua_isfunction(vm, index);

            // Setters //
            void setGlobal(std::string index) return lua_setglobal(vm, index.c_str());
            void setNil() return lua_pushnil(vm);
            void setBool(bool value) return lua_pushboolean(vm, static_cast<int>(value));
            void setString(std::string value) return lua_pushstring(vm, value.c_str());
            void setInt(int value) return lua_pushnumber(vm, (lua_Number)value);
            void setFloat(float value) return lua_pushnumber(vm, (lua_Number)value);
            void setLong(long value) return lua_pushnumber(vm, (lua_Number)value);
            void setDouble(double value) return lua_pushnumber(vm, (lua_Number)value);
            void setTable(int index = 1) return lua_settable(vm, index);
            void setTableField(int index = 1, std::string value) return lua_setfield(vm, index, value.c_str());
            void createMetaTable(std::string value) return luaL_newmetatable(vm, value.c_str());
            void setMetaTable(int index = 1) return lua_setmetatable(vm, index);
            void setMetaTable(std::string index) return luaL_setmetatable(vm, index.c_str());
            void createUserData(void* value) {
                void** userdata = static_cast<void**>(lua_newuserdata(vm, sizeof(void*)));
                *userdata = value;
                return;
            }
            void setUserData(void* value) return lua_pushlightuserdata(vm, value);
            void setFunction(lua_CFunction value) return lua_pushcfunction(vm, value);

            // Getters //
            int getArgCount() return lua_gettop(vm);
            bool getGlobal(std::string index) return lua_getglobal(vm, index.c_str());
            bool getBool(int index = 1) return static_cast<bool>(lua_toboolean(vm, index));
            std::string getString(int index = 1) return lua_tostring(vm, index) || std::string("");
            int getInt(int index = 1) return lua_tonumber(vm, index);
            float getFloat(int index = 1) return lua_tonumber(vm, index);
            double getDouble(int index = 1) return lua_tonumber(vm, index);
            long getLong(int index = 1) return lua_tonumber(vm, index);
            bool getTable(int index = 1) return lua_gettable(vm, index);
            bool getTableField(int index = 1, std::string value) return lua_getfield(vm, index, value.c_str());
            bool getMetaTable(int index = 1) return lua_getmetatable(vm, index);
            void* getUserData(int index = 1) return lua_touserdata(vm, index);

            // Registerers //
            void registerBool(std::string index, bool value) {
                setBool(value);
                return setTableField(-2, index.c_str());
            }
            void registerString(std::string index, std::string& value) {
                setString(value);
                return setTableField(-2, index.c_str());
            }
            void registerNumber(std::string index, int value) {
                setInt(value);
                return setTableField(-2, index.c_str());
            }
            void registerFunction(std::string index, lua_CFunction exec) {
                setFunction(exec);
                return setTableField(-2, index.c_str());
            }
            bool registerObject(std::string index, void* value) {
                createUserData(value);
                return setMetaTable(index);
            }

            // Utils //
            bool loadString(std::string& buffer) return luaL_loadstring(vm, buffer.c_str());
            bool throwError(std::string& error = "") {
                lua_Debug debug;
                lua_getstack(L, 1, &debug);
                lua_getinfo(L, "nSl", &debug);
                if (onErrorHandler) onErrorHandler("[ERROR - L" + std::to_string(debug.currentline) + "] | Reason: " + ((error.is_empty() && "N/A") || error));
                return true
            }
    };
}
