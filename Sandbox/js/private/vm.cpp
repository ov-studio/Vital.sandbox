/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: js: public: vm.cpp
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
// TODO: REMOVE LATER
#include <Sandbox/js/public/vm.h>
//#include <Sandbox/js/public/api.h>
#if __has_include(<Sandbox/js/module/bundle.h>)
    #include <Sandbox/js/module/bundle.h>
#endif


/////////////
// JS: VM //
/////////////

namespace Vital::JS {
    // Instantiators //
    std::map<vital_ref*, vital_vm*> vInstances;
    vital_vm* fetchVM(vital_ref* vm) { return vInstances[vm]; }
    create::create() {
        vm = duk_create_heap_default();
        vInstances.emplace(vm, this);
        for (std::string i : vBlacklist) {
            setNil();
            setGlobal(i);
        }
        /*
        for (auto& i : API::vMethods) {
            registerFunction(i.first.second, i.second, i.first.first);
        }
        */
        #if __has_include(<Sandbox/js/module/bundle.h>)
            for (std::string i : vBundle) {
                loadString(i);
            }
        #endif
    }
    bool create::destroy() {
        if (isUnloaded) return false;
        isUnloaded = true;
        duk_destroy_heap(vm);
        vm = nullptr;
        return true;
    }

    // Checkers //
    bool create::isNil(int index) { return duk_is_null_or_undefined(vm, index); }
    bool create::isBool(int index) { return duk_is_boolean(vm, index); }
    bool create::isString(int index) { return duk_is_string(vm, index); }
    bool create::isNumber(int index) { return duk_is_number(vm, index); }
    bool create::isArray(int index) { return duk_is_array(vm, index); }
    bool create::isObject(int index) { return duk_is_object(vm, index); }
    bool create::isThread(int index) { return duk_is_thread(vm, index); }
    bool create::isUserData(int index) { return duk_is_pointer(vm, index); }
    bool create::isFunction(int index) { return duk_is_function(vm, index); }

    // Setters //
    void create::setGlobal(std::string index) { duk_put_global_string(vm, index.data()); }
    void create::setNil() { duk_push_null(vm); }
    void create::setBool(bool value) { duk_push_boolean(vm, static_cast<int>(value)); }
    void create::setString(std::string& value) { duk_push_string(vm, value.data()); }
    void create::setInt(int value) { duk_push_number(vm, value); }
    void create::setFloat(float value) { duk_push_number(vm, value); }
    void create::setDouble(double value) { duk_push_number(vm, value); }
    void create::createArray() { duk_push_array(vm); }
    void create::createObject() { duk_push_object(vm); }
    void create::setTable(int index) { lua_settable(vm, index); }
    void create::setTableField(int value, int index) { lua_seti(vm, index, value); }
    void create::setTableField(std::string value, int index) { lua_setfield(vm, index, value.data()); }
    void create::createMetaTable(std::string value) { luaL_newmetatable(vm, value.data()); }
    void create::setMetaTable(int index) { lua_setmetatable(vm, index); }
    void create::setMetaTable(std::string index) { luaL_setmetatable(vm, index.data()); }
    void create::createUserData(void* value) {
        void** userdata = static_cast<void**>(lua_newuserdata(vm, sizeof(void*)));
        *userdata = value;
        return;
    }
    void create::setUserData(void* value) { duk_push_pointer(vm, value); }
    void create::setFunction(vital_exec& value) { duk_push_c_function(vm, value, DUK_VARARGS); }

    // Getters //
    int create::getArgCount() { return duk_get_top(vm); }
    bool create::getGlobal(std::string index) { return duk_get_global_string(vm, index.data()); }
    bool create::getBool(int index) { return static_cast<bool>(duk_to_boolean(vm, index)); }
    std::string create::getString(int index) { return duk_to_string(vm, index); }
    int create::getInt(int index) { return static_cast<int>(duk_to_number(vm, index)); }
    float create::getFloat(int index) { return static_cast<float>(duk_to_number(vm, index)); }
    double create::getDouble(int index) { return static_cast<double>(duk_to_number(vm, index)); }
    bool create::getTable(int index) { return lua_gettable(vm, index); }
    bool create::getTableField(int value, int index) { return lua_geti(vm, index, value); }
    bool create::getTableField(std::string value, int index) { return lua_getfield(vm, index, value.data()); }
    bool create::getMetaTable(int index) { return lua_getmetatable(vm, index); }
    void* create::getUserData(int index) { return duk_to_pointer(vm, index); }
    int create::getLength(int index) {
        duk_get_length(vm, index);
        int result = getInt(-1);
        pop();
        return result;
    }

    // Pushers //
    void create::pushBool(bool value) {
        setBool(value);
        setTableField(getLength(-2) + 1, -2);
    }
    void create::pushString(std::string& value) {
        setString(value);
        setTableField(getLength(-2) + 1, -2);
    }
    void create::pushNumber(int value) {
        setInt(value);
        setTableField(getLength(-2) + 1, -2);
    }
    void create::pushFunction(vital_exec& exec) {
        setFunction(exec);
        setTableField(getLength(-2) + 1, -2);
    }

    // Registerers //
    void create::registerBool(std::string index, bool value) {
        setBool(value);
        setTableField(index.data(), -2);
    }
    void create::registerString(std::string index, std::string& value) {
        setString(value);
        setTableField(index.data(), -2);
    }
    void create::registerNumber(std::string index, int value) {
        setInt(value);
        setTableField(index.data(), -2);
    }
    void create::registerFunction(std::string index, vital_exec& exec) {
        setFunction(exec);
        setTableField(index.data(), -2);
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
    void create::registerObject(std::string index, void* value) {
        createUserData(value);
        setMetaTable(index);
    }

    // Utils //
    void create::pop(int count) { duk_pop_n(vm, count); }
    bool create::loadString(std::string& buffer) {
        if (buffer.empty()) return false;
        duk_eval_string(vm, buffer.data());
        bool status = !lua_pcall(vm, 0, LUA_MULTRET, 0);
        if (!status) {
            std::string error = getString(-1);
            API::error(error);
            return false;
        }
        return true;
    }
    bool create::throwError(std::string& error) {
        lua_Debug debug;
        lua_getstack(vm, 1, &debug);
        lua_getinfo(vm, "nSl", &debug);
        error = "[ERROR - L" + std::to_string(debug.currentline) + "] | Reason: " + (error.empty() ? "N/A" : error);
        API::error(error);
        return true;
    }
}