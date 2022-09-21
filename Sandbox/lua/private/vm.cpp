/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: public: vm.cpp
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
#include <filesystem>
#include <Sandbox/lua/public/index.h>
#include <Sandbox/lua/public/vm.h>


//////////////
// Lua: VM //
//////////////

namespace Vital::Lua {
    // Instantiators //
    create::create() {
        vm = luaL_newstate();
        vInstances.emplace(vm, this);
        for (luaL_Reg i : vLibraries) {
            if (i.func) {
                luaL_requiref(vm, i.name, i.func, 1);
                lua_pop(vm, 1);
            }
        }
        for (std::string i : vBlacklist) {
            setNil();
            setGlobal(i);
        }
        for (auto& i : vMethods) {
            registerFunction(i.first.second, i.second, i.first.first);
        }
        for (std::string i : vModules) {
            std::string path = std::filesystem::current_path().string() + "/modules/" + i;
            std::string buffer = Vital::FileSystem::read(path);
            loadString(buffer);
        }
    }
    bool create::destroy() {
        if (isUnloaded) return false;
        isUnloaded = true;
        lua_close(vm);
        vm = nullptr;
        return true;
    }

    // Checkers //
    inline bool create::isNil(int index) {return lua_isnoneornil(vm, index);}
    inline bool create::isBool(int index) {return lua_isboolean(vm, index);}
    inline bool create::isString(int index) {return lua_isstring(vm, index);}
    inline bool create::isNumber(int index) {return lua_isnumber(vm, index);}
    inline bool create::isTable(int index) {return lua_istable(vm, index);}
    inline bool create::isThread(int index) {return lua_isthread(vm, index);}
    inline bool create::isUserData(int index) {return lua_isuserdata(vm, index);}
    inline bool create::isFunction(int index) {return lua_isfunction(vm, index);}

    // Setters //
    inline void create::setGlobal(std::string index) {lua_setglobal(vm, index.c_str());}
    inline void create::setNil() {lua_pushnil(vm);}
    inline void create::setBool(bool value) {lua_pushboolean(vm, static_cast<int>(value));}
    inline void create::setString(std::string value) {lua_pushstring(vm, value.c_str());}
    inline void create::setInt(int value) {lua_pushnumber(vm, (lua_Number)value);}
    inline void create::setFloat(float value) {lua_pushnumber(vm, (lua_Number)value);}
    inline void create::setDouble(double value) {lua_pushnumber(vm, (lua_Number)value);}
    inline void create::createTable() {lua_newtable(vm);}
    inline void create::setTable(int index) {lua_settable(vm, index);}
    inline void create::setTableField(int index, std::string value) {lua_setfield(vm, index, value.c_str());}
    inline void create::createMetaTable(std::string value) {luaL_newmetatable(vm, value.c_str());}
    inline void create::setMetaTable(int index) {lua_setmetatable(vm, index);}
    inline void create::setMetaTable(std::string index) {luaL_setmetatable(vm, index.c_str());}
    inline void create::createUserData(void* value) {
        void** userdata = static_cast<void**>(lua_newuserdata(vm, sizeof(void*)));
        *userdata = value;
        return;
    }
    inline void create::setUserData(void* value) {lua_pushlightuserdata(vm, value);}
    inline void create::setFunction(vital_exec& value) {lua_pushcfunction(vm, value);}

    // Getters //
    inline int create::getArgCount() {return lua_gettop(vm);}
    inline bool create::getGlobal(std::string index) {return lua_getglobal(vm, index.c_str());}
    inline bool create::getBool(int index) {return static_cast<bool>(lua_toboolean(vm, index));}
    inline std::string create::getString(int index) {return lua_tostring(vm, index);}
    inline int create::getInt(int index) {return static_cast<int>(lua_tonumber(vm, index));}
    inline float create::getFloat(int index) {return static_cast<float>(lua_tonumber(vm, index));}
    inline double create::getDouble(int index) {return static_cast<double>(lua_tonumber(vm, index));}
    inline bool create::getTable(int index) {return lua_gettable(vm, index);}
    inline bool create::getTableField(int index, std::string value) {return lua_getfield(vm, index, value.c_str());}
    inline bool create::getMetaTable(int index) {return lua_getmetatable(vm, index);}
    inline void* create::getUserData(int index) {return lua_touserdata(vm, index);}

    // Registerers //
    inline void create::registerBool(std::string index, bool value) {
        setBool(value);
        setTableField(-2, index.c_str());
    }
    inline void create::registerString(std::string index, std::string& value) {
        setString(value);
        setTableField(-2, index.c_str());
    }
    inline void create::registerNumber(std::string index, int value) {
        setInt(value);
        setTableField(-2, index.c_str());
    }
    inline void create::registerFunction(std::string index, vital_exec& exec) {
        setFunction(exec);
        setTableField(-2, index.c_str());
    }
    inline void create::registerFunction(std::string index, vital_exec& exec, std::string parent) {
        getGlobal(parent);
        if (!isTable(-1)) {
            createTable();
            setGlobal(parent);
            getGlobal(parent);
        }
        registerFunction(index, exec);
    }
    inline bool create::registerObject(std::string index, void* value) {
        createUserData(value);
        setMetaTable(index);
        return true;
    }

    // Utils //
    inline bool create::loadString(std::string& buffer) {return luaL_loadstring(vm, buffer.c_str());}
    inline bool create::throwError(std::string& error) {
        lua_Debug debug;
        lua_getstack(vm, 1, &debug);
        lua_getinfo(vm, "nSl", &debug);
        error = "[ERROR - L" + std::to_string(debug.currentline) + "] | Reason: " + (error.empty() ? "N/A" : error);
        if (onError) onError(error);
        return true;
    }

    // Method Binders //
    template<typename lambda_exec>
    bool bind(std::string parent, std::string name, lambda_exec exec) {
        const vital_exec_ref ref = vital_exec_ref {parent, name};
        if (vMethods[ref] && (vMethods[ref] == exec)) return false;
        vMethods.emplace(ref, [exec](lua_State* vm) -> int {
            return exec(vInstances[vm]);
        });
        return true;
    }
    template<typename lambda_exec>
    bool unbind(std::string parent, std::string name) {
        const vital_exec_ref ref = vital_exec_ref {parent, name};
        if (vMethods.find(ref) == vMethods.end()) return false;
        vMethods.erase(ref);
        return true;
    }
}