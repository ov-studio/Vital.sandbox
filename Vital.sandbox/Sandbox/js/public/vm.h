/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: js: public: vm.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: VM Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Sandbox/js/public/index.h>


/////////////
// JS: VM //
/////////////

namespace Vital::Sandbox::JS {
    typedef duk_context vsdk_ref;
    typedef duk_c_function vsdk_exec;
    typedef std::vector<std::pair<std::function<void(void*)>, std::function<void(void*)>>> vsdk_api;
    class create {
        private:
            vsdk_ref* vm = nullptr;
            vsdk_api api = {};
        public:
            // Instantiators //
            create(vsdk_api api = {});
            ~create();

            // Checkers //
            bool isNil(int index = 1);
            bool isBool(int index = 1);
            bool isString(int index = 1);
            bool isNumber(int index = 1);
            bool isArray(int index = 1);
            bool isObject(int index = 1);
            bool isThread(int index = 1);
            bool isUserData(int index = 1);
            bool isFunction(int index = 1);

            // Setters //
            void setGlobal(const std::string& index);
            void setNil();
            void setBool(bool value);
            void setString(const std::string& value);
            void setNumber(int value);
            void setNumber(float value);
            void setNumber(double value);
            void createArray();
            void createObject();
            void setArray(int index = 1);
            void setObject(int index = 1);
            void setArrayField(int value, int index = 1);
            void setObjectField(const std::string& value, int index = 1);
            void createNamespace(const std::string& parent);
            void setUserData(void* value);
            void setFunction(vsdk_exec& value);

            // Getters //
            int getArgCount();
            bool getGlobal(const std::string& index);
            bool getBool(int index = 1);
            std::string getString(int index = 1);
            int getInt(int index = 1);
            float getFloat(int index = 1);
            double getDouble(int index = 1);
            bool getArray(int index = 1);
            bool getObject(int index = 1);
            bool getArrayField(int value, int index = 1);
            bool getObjectField(const std::string& value, int index = 1);
            void* getUserData(int index = 1);
            int getLength(int index = 1);

            // Pushers //
            void pushBool(bool value);
            void pushString(const std::string& value);
            void pushNumber(int value);
            void pushNumber(float value);
            void pushNumber(double value);
            void pushFunction(vsdk_exec& exec);

            // Registerers //
            void registerBool(const std::string& index, bool value);
            void registerBool(const std::string& index, bool value, const std::string& parent);
            void registerString(const std::string& index, const std::string& value);
            void registerString(const std::string& index, const std::string& value, const std::string& parent);
            void registerNumber(const std::string& index, int value);
            void registerNumber(const std::string& index, int value, const std::string& parent);
            void registerNumber(const std::string& index, float value);
            void registerNumber(const std::string& index, float value, const std::string& parent);
            void registerNumber(const std::string& index, double value);
            void registerNumber(const std::string& index, double value, const std::string& parent);
            void registerFunction(const std::string& index, vsdk_exec& exec);
            void registerFunction(const std::string& index, vsdk_exec& exec, const std::string& parent);

            // Utils //
            void pop(int count = 1);
            void hook(const std::string& mode);
            int execute(std::function<int()> exec);
            bool loadString(const std::string& buffer);
            void throwError(const std::string& error = "");
    };
    typedef create vsdk_vm;
    typedef std::map<vsdk_ref*, vsdk_vm*> vsdk_vms;
    extern vsdk_vms fetchVMs();
    extern vsdk_vm* fetchVM(vsdk_ref* vm);
}
