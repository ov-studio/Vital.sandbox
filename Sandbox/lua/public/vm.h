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
#include <pch.h>
#include <Sandbox/lua/public/index.h>


//////////////
// Lua: VM //
//////////////

namespace Vital::Lua {
    typedef lua_CFunction vital_exec;
    typedef std::pair<std::string, std::string> vital_exec_ref;
    static std::map<vital_exec_ref, vital_exec&> vMethods;

    const class create {
        private:
            bool isUnloaded = false;
            lua_State* vm = nullptr;
        public:
            create();
            bool destroy();

            // Checkers //
            bool isNil(int index);
            bool isBool(int index);
            bool isString(int index);
            bool isNumber(int index);
            bool isTable(int index);
            bool isThread(int index);
            bool isUserData(int index);
            bool isFunction(int index);

            // Setters //
            void setGlobal(std::string index);
            void setNil();
            void setBool(bool value);
            void setString(std::string value);
            void setInt(int value);
            void setFloat(float value);
            void setDouble(double value);
            void createTable();
            void setTable(int index = 1);
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
            bool getTableField(std::string value, int index = 1);
            bool getMetaTable(int index = 1);
            void* getUserData(int index = 1);

            // Registerers //
            void registerBool(std::string index, bool value);
            void registerString(std::string index, std::string& value);
            void registerNumber(std::string index, int value);
            void registerFunction(std::string index, vital_exec& exec);
            void registerFunction(std::string index, vital_exec& exec, std::string parent);
            bool registerObject(std::string index, void* value);

            // Utils //
            bool loadString(std::string& buffer);
            bool throwError(std::string& error);
    };
    typedef create vital_vm;
    static std::map<lua_State*, vital_vm*> vInstances;

    // Method Binders //
    static std::function<void(std::string&)> onError = NULL;
    template<typename lambda_exec>
    extern bool bind(std::string parent, std::string name, lambda_exec exec);
    extern bool unbind(std::string parent, std::string name);
}