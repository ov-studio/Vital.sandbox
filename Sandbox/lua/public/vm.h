/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: public: vm.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: VM Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <map>
#include <functional>
#include "Sandbox/lua/index.h"


//////////////
// Lua: VM //
//////////////

namespace Vital::Lua {
    const class create {
        private:
            bool isUnloaded = false;
            lua_State* vm = nullptr;
        public:
            create() {
                vm = luaL_newstate();
                vInstances.insert_or_assign(vm, this);
                for (int i = 0; i < sizeof(Library_Whitelist); i++) {
                    const luaL_Reg* j = Library_Whitelist[i];
                    if (j -> funct) {
                        luaL_requiref(vm, j -> name, j -> func, 1);
                        lua_pop(vm, 1);
                    }
                }
                for (auto& i : vBlacklist) {
                    setNil();
                    setGlobal(i);
                }
                for (auto& i : vMethods) {
                    registerFunction(i.first.second, i.second, i.first.first);
                }
                for (auto& i : vModules) {
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
            void createTable() return lua_newtable(vm);
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
            void setFunction(vital_exec& value) return lua_pushcfunction(vm, value);

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
            void registerFunction(std::string index, vital_exec& exec) {
                setFunction(exec);
                return setTableField(-2, index.c_str());
            }
            void registerFunction(std::string index, vital_exec& exec, std::string parent) {
                getGlobal(parent)
                if (!isTable(-1)) {
                    createTable();
                    setGlobal(parent);
                    getGlobal(parent);
                }
                return registerFunction(index, exec);
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
                if (onError) onError("[ERROR - L" + std::to_string(debug.currentline) + "] | Reason: " + ((error.is_empty() && "N/A") || error));
                return true
            }
    };
    typedef create vital_vm;
    typedef lua_CFunction vital_exec;
    typedef std::pair<std::string, std::string> vital_exec_ref;
    static const std::map<lua_State*, vital_vm*> vInstances;
    static const std::map<vital_exec_ref, vital_exec&> vMethods;

    // Method Binders //
    static std::function<void(std::string&)> onError = NULL;
    static const bool bind(std::string parent, std::string name, std::function<void(vital_vm*)> exec) {
        const vital_exec_ref ref = vital_exec_ref {parent, name};
        if (vMethodRefs[ref] && (vMethodRefs[ref] == exec)) return false;
        vMethods.insert_or_assign(ref, [](luastate* vm) {
            exec(vInstances[vm]);
        });
        return true
    }
    static const bool unbind(std::string parent, std::string name) {
        const vital_exec_ref ref = vital_exec_ref {parent, name};
        if (!vMethodRefs[ref]) return false;
        vMethods.erase(ref);
        return true
    }
}