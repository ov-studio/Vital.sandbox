/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: js: public: vm.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
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

namespace Vital::JS {
    typedef duk_context vital_ref;
    typedef duk_c_function vital_exec;
    typedef std::pair<std::string, std::string> vital_exec_ref;
    class create {
        private:
            bool isUnloaded = false;
            vital_ref* vm = nullptr;
        public:
            // Instantiators //
            create();
            bool destroy();

            // Checkers //
            bool isNil(int index);
            bool isBool(int index);
            bool isString(int index);
            bool isNumber(int index);
            bool isArray(int index);
            bool isObject(int index);
            bool isThread(int index);
            bool isUserData(int index);
            bool isFunction(int index);

            // Setters //
            void setGlobal(std::string index);
            void setNil();
            void setBool(bool value);
            void setString(std::string& value);
            void setInt(int value);
            void setFloat(float value);
            void setDouble(double value);
            void createArray();
            void createObject();
            void setTable(int index = 1);
            void setTableField(int value, int index = 1);
            void setTableField(std::string value, int index = 1);
            void createMetaTable(std::string value);
            void setMetaTable(int index = 1);
            void setMetaTable(std::string index);
            void createUserData(void* value);
            void setUserData(void* value);
            void setFunction(vital_exec& value);

            // Getters //
            int getArgCount();
            bool getGlobal(std::string index);
            bool getBool(int index = 1);
            std::string getString(int index = 1);
            int getInt(int index = 1);
            float getFloat(int index = 1);
            double getDouble(int index = 1);
            bool getTable(int index = 1);
            bool getTableField(int value, int index = 1);
            bool getTableField(std::string value, int index = 1);
            bool getMetaTable(int index = 1);
            void* getUserData(int index = 1);
            int getLength(int index = 1);

            // Pushers //
            void pushBool(bool value);
            void pushString(std::string& value);
            void pushNumber(int value);
            void pushFunction(vital_exec& exec);

            // Registerers //
            void registerBool(std::string index, bool value);
            void registerString(std::string index, std::string& value);
            void registerNumber(std::string index, int value);
            void registerFunction(std::string index, vital_exec& exec);
            void registerFunction(std::string index, vital_exec& exec, std::string parent);
            void registerObject(std::string index, void* value);

            // Utils //
            void pop(int count = 1);
            bool loadString(std::string& buffer);
            bool throwError(std::string& error);
    };
    typedef create vital_vm;
    extern vital_vm* fetchVM(vital_ref* vm);
}