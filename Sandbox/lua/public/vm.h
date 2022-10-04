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
#include <Sandbox/lua/public/index.h>


//////////////
// Lua: VM //
//////////////

namespace Vital::Sandbox::Lua {
    typedef lua_State vital_ref;
    typedef lua_CFunction vital_exec;
    typedef std::pair<std::string, std::string> vital_exec_ref;
    class create {
        private:
            vital_ref* vm = nullptr;
        public:
            // Instantiators //
            create();
            bool destroy();

            // Checkers //
            bool isNil(int index = 1);
            bool isBool(int index = 1);
            bool isString(int index = 1);
            bool isNumber(int index = 1);
            bool isTable(int index = 1);
            bool isThread(int index = 1);
            bool isUserData(int index = 1);
            bool isFunction(int index = 1);

            // Setters //
            void setGlobal(std::string index);
            void setNil();
            void setBool(bool value);
            void setString(std::string& value);
            void setNumber(int value);
            void setNumber(float value);
            void setNumber(double value);
            void createTable();
            void setTable(int index = 1);
            void setTableField(int value, int index = 1);
            void setTableField(std::string value, int index = 1);
            void createMetaTable(std::string value);
            void setMetaTable(int index = 1);
            void setMetaTable(std::string index);
            void createNamespace(std::string parent);
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
            bool getMetaTable(std::string index);
            void* getUserData(int index = 1);
            int getLength(int index = 1);

            // Pushers //
            void pushBool(bool value);
            void pushString(std::string& value);
            void pushNumber(int value);
            void pushNumber(float value);
            void pushNumber(double value);
            void pushFunction(vital_exec& exec);

            // Registerers //
            void registerBool(std::string index, bool value);
            void registerBool(std::string index, bool value, std::string parent);
            void registerString(std::string index, std::string& value);
            void registerString(std::string index, std::string& value, std::string parent);
            void registerNumber(std::string index, int value);
            void registerNumber(std::string index, int value, std::string parent);
            void registerNumber(std::string index, float value);
            void registerNumber(std::string index, float value, std::string parent);
            void registerNumber(std::string index, double value);
            void registerNumber(std::string index, double value, std::string parent);
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
