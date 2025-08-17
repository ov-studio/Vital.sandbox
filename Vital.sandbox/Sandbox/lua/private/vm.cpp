/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: public: vm.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: VM Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/file.h>
#include <Sandbox/lua/public/api.h>
#include <godot_cpp/variant/utility_functions.hpp>


//////////////////////////
// Vital: Sandbox: Lua //
//////////////////////////

namespace Vital::Sandbox::Lua {
    // Instantiators //
    vsdk_vms vms;
    create::create(vsdk_apis apis) {
        vm = luaL_newstate();
        this -> apis = apis;
        vms.emplace(vm, this);
        for (auto& i : vsdk_libraries) {
            luaL_requiref(vm, i.name, i.func, 1);
            pop();
        }
        for (auto& i : vsdk_blacklist) {
            setNil();
            setGlobal(i);
        }
        this -> apis = apis;
        this -> hook("bind");
        for (auto& i : Vital::Sandbox::fetchModules("lua")) {
            loadString(i);
        }
        this -> hook("inject");
    }
    create::create(vsdk_ref* thread) {
        vm = thread;
        this -> thread = true;
        vms.emplace(vm, this);
    }
    create::~create() {
        if (!vm) return;
        vms.erase(vm);
        vm = nullptr;
    }
    vsdk_vms fetchVMs() { return vms; }
    vsdk_vm* fetchVM(vsdk_ref* vm) { return vms.find(vm) != vms.end() ? vms.at(vm) : nullptr; }

    // Checkers //
    bool create::isVirtualThread() { return thread; }
    bool create::isNil(int index) { return lua_isnoneornil(vm, index); }
    bool create::isBool(int index) { return lua_isboolean(vm, index); }
    bool create::isString(int index) { return lua_isstring(vm, index); }
    bool create::isNumber(int index) { return lua_isnumber(vm, index); }
    bool create::isTable(int index) { return lua_istable(vm, index); }
    bool create::isThread(int index) { return lua_isthread(vm, index); }
    bool create::isUserData(int index) { return lua_isuserdata(vm, index); }
    bool create::isFunction(int index) { return lua_isfunction(vm, index); }
    bool create::isReference(const std::string& name) { return reference.at(name) ? true : false; }

    // Setters //
    void create::setGlobal(const std::string& index) { lua_setglobal(vm, index.data()); }
    void create::setNil() { lua_pushnil(vm); }
    void create::setBool(bool value) { lua_pushboolean(vm, static_cast<int>(value)); }
    void create::setString(const std::string& value) { lua_pushstring(vm, value.data()); }
    void create::setNumber(int value) { lua_pushnumber(vm, static_cast<lua_Number>(value)); }
    void create::setNumber(float value) { lua_pushnumber(vm, static_cast<lua_Number>(value)); }
    void create::setNumber(double value) { lua_pushnumber(vm, value); }
    void create::createTable() { lua_newtable(vm); }
    void create::setTable(int index) { lua_settable(vm, index); }
    void create::setTableField(int value, int index) { lua_seti(vm, index, value); }
    void create::setTableField(const std::string& value, int index) { lua_setfield(vm, index, value.data()); }
    void create::createMetaTable(const std::string& value) { luaL_newmetatable(vm, value.data()); }
    void create::setMetaTable(int index) { lua_setmetatable(vm, index); }
    void create::setMetaTable(const std::string& index) { luaL_setmetatable(vm, index.data()); }
    void create::createNamespace(const std::string& parent) {
        getGlobal(parent);
        if (!isTable(-1)) {
            pop();
            createTable();
            setGlobal(parent);
            getGlobal(parent);
        }
    }
    create* create::createThread() { return new create(lua_newthread(vm)); }
    void create::createUserData(void* value) {
        void** userdata = static_cast<void**>(lua_newuserdata(vm, sizeof(void*)));
        *userdata = value;
    }
    void create::setUserData(void* value) { lua_pushlightuserdata(vm, value); }
    void create::setFunction(vsdk_exec& value) { lua_pushcfunction(vm, value); }
    void create::setReference(const std::string& name, int index) {
        push(index);
        reference.emplace(name, luaL_ref(vm, LUA_REGISTRYINDEX));
    }

    // Getters //
    int create::getArgCount() { return lua_gettop(vm); }
    bool create::getGlobal(const std::string& index) { return lua_getglobal(vm, index.data()); }
    bool create::getBool(int index) { return static_cast<bool>(lua_toboolean(vm, index)); }
    std::string create::getString(int index) { return lua_tostring(vm, index); }
    int create::getInt(int index) { return static_cast<int>(lua_tonumber(vm, index)); }
    float create::getFloat(int index) { return static_cast<float>(lua_tonumber(vm, index)); }
    double create::getDouble(int index) { return static_cast<double>(lua_tonumber(vm, index)); }
    bool create::getTable(int index) { return lua_gettable(vm, index); }
    bool create::getTableField(int value, int index) { return lua_geti(vm, index, value); }
    bool create::getTableField(const std::string& value, int index) { return lua_getfield(vm, index, value.data()); }
    bool create::getMetaTable(int index) { return lua_getmetatable(vm, index); }
    bool create::getMetaTable(const std::string& index) { return luaL_getmetatable(vm, index.data()); }
    vsdk_ref* create::getThread(int index) { return lua_tothread(vm, index); }
    void* create::getUserData(int index) { return lua_touserdata(vm, index); }
    int create::getReference(const std::string& name, bool pushValue) {
        if (!pushValue) return reference.at(name);
        lua_rawgeti(vm, LUA_REGISTRYINDEX, reference.at(name));
        return 0;
    }
    int create::getLength(int index) {
        lua_len(vm, index);
        int result = getInt(-1);
        pop();
        return result;
    }

    // Pushers //
    void create::pushNil() {
        setNil();
        setTableField(getLength(-2) + 1, -2);
    }
    void create::pushBool(bool value) {
        setBool(value);
        setTableField(getLength(-2) + 1, -2);
    }
    void create::pushString(const std::string& value) {
        setString(value);
        setTableField(getLength(-2) + 1, -2);
    }
    void create::pushNumber(int value) {
        setNumber(value);
        setTableField(getLength(-2) + 1, -2);
    }
    void create::pushNumber(float value) {
        setNumber(value);
        setTableField(getLength(-2) + 1, -2);
    }
    void create::pushNumber(double value) {
        setNumber(value);
        setTableField(getLength(-2) + 1, -2);
    }
    void create::pushTable() {
        if (!isTable(-1)) return;
        setTableField(getLength(-2) + 1, -2);
    }
    void create::pushFunction(vsdk_exec& exec) {
        setFunction(exec);
        setTableField(getLength(-2) + 1, -2);
    }

    // Registerers //
    void create::registerNil(const std::string& index) {
        setNil();
        setTableField(index.data(), -2);
    }
    void create::registerNil(const std::string& index, const std::string& parent) {
        createNamespace(parent);
        registerNil(index);
        pop();
    }
    void create::registerBool(const std::string& index, bool value) {
        setBool(value);
        setTableField(index.data(), -2);
    }
    void create::registerBool(const std::string& index, bool value, const std::string& parent) {
        createNamespace(parent);
        registerBool(index, value);
        pop();
    }
    void create::registerString(const std::string& index, const std::string& value) {
        setString(value);
        setTableField(index.data(), -2);
    }
    void create::registerString(const std::string& index, const std::string& value, const std::string& parent) {
        createNamespace(parent);
        registerString(index, value);
        pop();
    }
    void create::registerNumber(const std::string& index, int value) {
        setNumber(value);
        setTableField(index.data(), -2);
    }
    void create::registerNumber(const std::string& index, int value, const std::string& parent) {
        createNamespace(parent);
        registerNumber(index, value);
        pop();
    }
    void create::registerNumber(const std::string& index, float value) {
        setNumber(value);
        setTableField(index.data(), -2);
    }
    void create::registerNumber(const std::string& index, float value, const std::string& parent) {
        createNamespace(parent);
        registerNumber(index, value);
        pop();
    }
    void create::registerNumber(const std::string& index, double value) {
        setNumber(value);
        setTableField(index.data(), -2);
    }
    void create::registerNumber(const std::string& index, double value, const std::string& parent) {
        createNamespace(parent);
        registerNumber(index, value);
        pop();
    }
    void create::registerTable(const std::string& index) {
        if (!isTable(-1)) return;
        setTableField(index.data(), -2);
    }
    void create::registerTable(const std::string& index, const std::string& parent) {
        if (!isTable(-1)) return;
        createNamespace(parent);
        registerTable(index);
        pop();
    }
    void create::registerFunction(const std::string& index, vsdk_exec& exec) {
        setFunction(exec);
        setTableField(index.data(), -2);
    }
    void create::registerFunction(const std::string& index, vsdk_exec& exec, const std::string& parent) {
        createNamespace(parent);
        registerFunction(index, exec);
        pop();
    }
    void create::registerObject(const std::string& index, void* value) {
        createUserData(value);
        setMetaTable(index);
    }

    // Utils //
    void create::push(int index) { lua_pushvalue(vm, index); }
    void create::pop(int count) { lua_pop(vm, count); }
    void create::move(vsdk_vm* target, int count) { lua_xmove(vm, target -> vm, count); }
    bool create::pcall(int arguments, int returns) { return lua_pcall(vm, arguments, returns, 0); }
    void create::removeReference(const std::string& name) {
        if (!isReference(name)) return;
        luaL_unref(vm, LUA_REGISTRYINDEX, getReference(name));
        reference.erase(name);
    }
    void create::resume(int count) {
        if (!isVirtualThread()) return;
        int ncount;
        lua_resume(vm, nullptr, count, &ncount);
        if (lua_status(vm) != LUA_YIELD) delete this;
    }
    void create::pause(int count) {
        if (!isVirtualThread()) return;
        lua_yield(vm, count);
    }
    int create::execute(std::function<int()> exec) {
        try { return exec(); }
        catch(const std::runtime_error& error) { throwError(error.what()); }
        catch(...) { throwError(); }
        return 1;
    }
    bool create::loadString(const std::string& buffer, bool autoload) {
        if (buffer.empty()) return false;
        if (!autoload) {
            const std::string __buffer = "return (function() " + buffer + " end)";
            luaL_loadstring(vm, __buffer.data());
        }
        else luaL_loadstring(vm, buffer.data());
        bool status = pcall(0, LUA_MULTRET);
        if (status) {
            API::error(getString(-1));
            pop();
            return false;
        }
        return true;
    }
    void create::throwError(const std::string& error) {
        lua_Debug debug;
        lua_getstack(vm, 1, &debug);
        lua_getinfo(vm, "nSl", &debug);
        API::error("[ERROR - L" + std::to_string(debug.currentline) + "] | Reason: " + (error.empty() ? "N/A" : error));
        setBool(false);
    }
}