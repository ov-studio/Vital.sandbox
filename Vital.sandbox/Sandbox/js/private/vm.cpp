/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: js: public: vm.cpp
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
#include <Sandbox/js/public/api.h>
#if __has_include(<Sandbox/js/module/bundle.h>)
    #include <Sandbox/js/module/bundle.h>
#endif


/////////////
// JS: VM //
/////////////

namespace Vital::Sandbox::JS {
    std::map<vsdk_ref*, vsdk_vm*> vsdk_vms;

    // Instantiators //
    create::create() {
        vm = duk_create_heap_default();
        vsdk_vms.emplace(vm, this);
        for (const std::string& i : vsdk_blacklist) {
            setNil();
            setGlobal(i);
        }
        API::boot(this);
        #if __has_include(<Sandbox/js/module/bundle.h>)
            for (const std::string& i : vsdk_modules) {
                loadString(fetchPackageModule(i));
            }
        #endif
    }
    create::~create() {
        if (!vm) return;
        vsdk_vms.erase(vm);
        duk_destroy_heap(vm);
        vm = nullptr;
    }
    vsdk_vm* fetchVM(vsdk_ref* vm) {
        return vsdk_vms.find(vm) != vsdk_vms.end() ? vsdk_vms.at(vm) : nullptr;
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
    void create::setGlobal(const std::string& index) { duk_put_global_string(vm, index.data()); }
    void create::setNil() { duk_push_null(vm); }
    void create::setBool(bool value) { duk_push_boolean(vm, static_cast<int>(value)); }
    void create::setString(const std::string& value) { duk_push_string(vm, value.data()); }
    void create::setNumber(int value) { duk_push_number(vm, value); }
    void create::setNumber(float value) { duk_push_number(vm, value); }
    void create::setNumber(double value) { duk_push_number(vm, value); }
    void create::createArray() { duk_push_array(vm); }
    void create::createObject() { duk_push_object(vm); }
    void create::setArray(int index) { duk_put_prop(vm, index); }
    void create::setObject(int index) { duk_put_prop(vm, index); }
    void create::setArrayField(int value, int index) { duk_put_prop_index(vm, index, value); }
    void create::setObjectField(const std::string& value, int index) { duk_put_prop_string(vm, index, value.data()); }
    void create::createNamespace(const std::string& parent) {
        getGlobal(parent);
        if (!isObject(-1)) {
            pop();
            createObject();
            setGlobal(parent);
            getGlobal(parent);
        }
    }
    void create::setUserData(void* value) { duk_push_pointer(vm, value); }
    void create::setFunction(vsdk_exec& value) { duk_push_c_function(vm, value, DUK_VARARGS); }

    // Getters //
    int create::getArgCount() { return duk_get_top(vm); }
    bool create::getGlobal(const std::string& index) { return duk_get_global_string(vm, index.data()); }
    bool create::getBool(int index) { return static_cast<bool>(duk_to_boolean(vm, index)); }
    std::string create::getString(int index) { return duk_to_string(vm, index); }
    int create::getInt(int index) { return static_cast<int>(duk_to_number(vm, index)); }
    float create::getFloat(int index) { return static_cast<float>(duk_to_number(vm, index)); }
    double create::getDouble(int index) { return static_cast<double>(duk_to_number(vm, index)); }
    bool create::getArray(int index) { return duk_get_prop(vm, index); }
    bool create::getObject(int index) { return duk_get_prop(vm, index); }
    bool create::getArrayField(int value, int index) { return duk_get_prop_index(vm, index, value); }
    bool create::getObjectField(const std::string& value, int index) { return duk_get_prop_string(vm, index, value.data()); }
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
        setArrayField(getLength(-2) + 1, -2);
    }
    void create::pushString(const std::string& value) {
        setString(value);
        setArrayField(getLength(-2) + 1, -2);
    }
    void create::pushNumber(int value) {
        setNumber(value);
        setArrayField(getLength(-2) + 1, -2);
    }
    void create::pushNumber(float value) {
        setNumber(value);
        setArrayField(getLength(-2) + 1, -2);
    }
    void create::pushNumber(double value) {
        setNumber(value);
        setArrayField(getLength(-2) + 1, -2);
    }
    void create::pushFunction(vsdk_exec& exec) {
        setFunction(exec);
        setArrayField(getLength(-2) + 1, -2);
    }

    // Registerers //
    void create::registerBool(const std::string& index, bool value) {
        setBool(value);
        setObjectField(index.data(), -2);
    }
    void create::registerBool(const std::string& index, bool value, const std::string& parent) {
        createNamespace(parent);
        registerBool(index, value);
        pop();
    }
    void create::registerString(const std::string& index, const std::string& value) {
        setString(value);
        setObjectField(index.data(), -2);
    }
    void create::registerString(const std::string& index, const std::string& value, const std::string& parent) {
        createNamespace(parent);
        registerString(index, value);
        pop();
    }
    void create::registerNumber(const std::string& index, int value) {
        setNumber(value);
        setObjectField(index.data(), -2);
    }
    void create::registerNumber(const std::string& index, int value, const std::string& parent) {
        createNamespace(parent);
        registerNumber(index, value);
        pop();
    }
    void create::registerNumber(const std::string& index, float value) {
        setNumber(value);
        setObjectField(index.data(), -2);
    }
    void create::registerNumber(const std::string& index, float value, const std::string& parent) {
        createNamespace(parent);
        registerNumber(index, value);
        pop();
    }
    void create::registerNumber(const std::string& index, double value) {
        setNumber(value);
        setObjectField(index.data(), -2);
    }
    void create::registerNumber(const std::string& index, double value, const std::string& parent) {
        createNamespace(parent);
        registerNumber(index, value);
        pop();
    }
    void create::registerFunction(const std::string& index, vsdk_exec& exec) {
        setFunction(exec);
        setObjectField(index.data(), -2);
    }
    void create::registerFunction(const std::string& index, vsdk_exec& exec, const std::string& parent) {
        createNamespace(parent);
        registerFunction(index, exec);
        pop();
    }

    // Utils //
    void create::pop(int count) { duk_pop_n(vm, count); }
    int create::execute(std::function<int()> exec) {
        try { return exec(); }
        catch(const std::runtime_error& error) { throwError(error.what()); }
        catch(...) { throwError(); }
        return 1;
    }
    bool create::loadString(const std::string& buffer) {
        if (buffer.empty()) return false;
        setString(buffer);
        bool status = !duk_peval(vm);
        if (!status) {
            API::error(duk_safe_to_string(vm, -1));
            pop();
            return false;
        }
        return true;
    }
    bool create::throwError(const std::string& error) {
        duk_inspect_callstack_entry(vm, -2);
        getObjectField("lineNumber", -1);
        API::error("[ERROR - L" + std::to_string(getInt(-1)) + "] | Reason: " + (error.empty() ? "N/A" : error));
        setBool(false);
        return true;
    }
}
