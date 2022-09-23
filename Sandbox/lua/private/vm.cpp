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
#include <System/public/filesystem.h>
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
            luaL_requiref(vm, i.name, i.func, 1);
            lua_pop(vm, 1);
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
    bool create::isNil(int index) { return lua_isnoneornil(vm, index); }
    bool create::isBool(int index) { return lua_isboolean(vm, index); }
    bool create::isString(int index) { return lua_isstring(vm, index); }
    bool create::isNumber(int index) { return lua_isnumber(vm, index); }
    bool create::isTable(int index) { return lua_istable(vm, index); }
    bool create::isThread(int index) { return lua_isthread(vm, index); }
    bool create::isUserData(int index) { return lua_isuserdata(vm, index); }
    bool create::isFunction(int index) { return lua_isfunction(vm, index); }

    // Setters //
    void create::setGlobal(std::string index) {lua_setglobal(vm, index.c_str()); }
    void create::setNil() {lua_pushnil(vm); }
    void create::setBool(bool value) {lua_pushboolean(vm, static_cast<int>(value)); }
    void create::setString(std::string value) {lua_pushstring(vm, value.c_str()); }
    void create::setInt(int value) {lua_pushnumber(vm, static_cast<lua_Number>(value)); }
    void create::setFloat(float value) {lua_pushnumber(vm, static_cast<lua_Number>(value)); }
    void create::setDouble(double value) {lua_pushnumber(vm, static_cast<lua_Number>(value)); }
    void create::createTable() {lua_newtable(vm); }
    void create::setTable(int index) {lua_settable(vm, index); }
    void create::setTableField(std::string value, int index) {lua_setfield(vm, index, value.c_str()); }
    void create::createMetaTable(std::string value) {luaL_newmetatable(vm, value.c_str()); }
    void create::setMetaTable(int index) {lua_setmetatable(vm, index); }
    void create::setMetaTable(std::string index) {luaL_setmetatable(vm, index.c_str()); }
    void create::createUserData(void* value) {
        void** userdata = static_cast<void**>(lua_newuserdata(vm, sizeof(void*)));
        *userdata = value;
        return;
    }
    void create::setUserData(void* value) {lua_pushlightuserdata(vm, value); }
    void create::setFunction(vital_exec& value) {lua_pushcfunction(vm, reinterpret_cast<lua_CFunction>(&value)); }

    // Getters //
    int create::getArgCount() { return lua_gettop(vm); }
    bool create::getGlobal(std::string index) { return lua_getglobal(vm, index.c_str()); }
    bool create::getBool(int index) { return static_cast<bool>(lua_toboolean(vm, index)); }
    std::string create::getString(int index) { return lua_tostring(vm, index); }
    int create::getInt(int index) { return static_cast<int>(lua_tonumber(vm, index)); }
    float create::getFloat(int index) { return static_cast<float>(lua_tonumber(vm, index)); }
    double create::getDouble(int index) { return static_cast<double>(lua_tonumber(vm, index)); }
    bool create::getTable(int index) { return lua_gettable(vm, index); }
    bool create::getTableField(std::string value, int index) { return lua_getfield(vm, index, value.c_str()); }
    bool create::getMetaTable(int index) { return lua_getmetatable(vm, index); }
    void* create::getUserData(int index) { return lua_touserdata(vm, index); }

    // Registerers //
    void create::registerBool(std::string index, bool value) {
        setBool(value);
        setTableField(index.c_str(), -2);
    }
    void create::registerString(std::string index, std::string& value) {
        setString(value);
        setTableField(index.c_str(), -2);
    }
    void create::registerNumber(std::string index, int value) {
        setInt(value);
        setTableField(index.c_str(), -2);
    }
    void create::registerFunction(std::string index, vital_exec& exec) {
        setFunction(exec);
        setTableField(index.c_str(), -2);
    }
    void create::registerFunction(std::string index, vital_exec& exec, std::string parent) {
        getGlobal(parent);
        if (!isTable(-1)) {
            createTable();
            setGlobal(parent);
            getGlobal(parent);
        }
        registerFunction(index, exec);
    }
    bool create::registerObject(std::string index, void* value) {
        createUserData(value);
        setMetaTable(index);
        return true;
    }

    // Utils //
    bool create::loadString(std::string& buffer) { return luaL_loadstring(vm, buffer.c_str()); }
    bool create::throwError(std::string& error) {
        lua_Debug debug;
        lua_getstack(vm, 1, &debug);
        lua_getinfo(vm, "nSl", &debug);
        error = "[ERROR - L" + std::to_string(debug.currentline) + "] | Reason: " + (error.empty() ? "N/A" : error);
        if (onError) onError(error);
        return true;
    }

    // Method Binders //
    bool bind(std::string parent, std::string name, std::function<int(vital_vm* vm)> exec) {
        vital_exec_ref ref = {parent, name};
        vMethods[ref] = [&](lua_State* vm) -> int {
            return exec(vInstances[vm]);
        };
        return true;
    }
    bool unbind(std::string parent, std::string name) {
        vital_exec_ref ref = {parent, name};
        vMethods.erase(ref);
        return true;
    }
}