/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: public: vm.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: VM Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Sandbox/lua/public/index.h>


//////////////////////////
// Vital: Sandbox: Lua //
//////////////////////////

namespace Vital::Sandbox::Lua {
    typedef lua_State vsdk_ref;
    typedef lua_CFunction vsdk_exec;
    typedef std::map<std::string, int> vsdk_reference;
    typedef std::vector<std::pair<std::function<void(void*)>, std::function<void(void*)>>> vsdk_apis;
    class create {
        private:
            vsdk_ref* vm = nullptr;
            vsdk_reference reference = {};
            vsdk_apis apis = {};
            bool thread = false;
        public:
            // Instantiators //
            create(vsdk_apis apis = {});
            create(vsdk_ref* thread);
            ~create();

            // Checkers //
            bool isVirtualThread();
            bool isNil(int index = 1);
            bool isBool(int index = 1);
            bool isString(int index = 1);
            bool isNumber(int index = 1);
            bool isTable(int index = 1);
            bool isThread(int index = 1);
            bool isUserData(int index = 1);
            bool isFunction(int index = 1);
            bool isReference(const std::string& name);

            // Setters //
            void setGlobal(const std::string& index);
            void setNil();
            void setBool(bool value);
            void setString(const std::string& value);
            void setNumber(int value);
            void setNumber(float value);
            void setNumber(double value);
            void createTable();
            void setTable(int index = 1);
            void setTableField(int value, int index = 1);
            void setTableField(const std::string& value, int index = 1);
            void createMetaTable(const std::string& value);
            void setMetaTable(int index = 1);
            void setMetaTable(const std::string& index);
            void createNamespace(const std::string& parent);
            create* createThread();
            void createUserData(void* value);
            void setUserData(void* value);
            void setFunction(vsdk_exec& value);
            void setReference(const std::string& name, int index = 1);

            // Getters //
            int getArgCount();
            bool getGlobal(const std::string& index);
            bool getBool(int index = 1);
            std::string getString(int index = 1);
            int getInt(int index = 1);
            float getFloat(int index = 1);
            double getDouble(int index = 1);
            bool getTable(int index = 1);
            bool getTableField(int value, int index = 1);
            bool getTableField(const std::string& value, int index = 1);
            bool getMetaTable(int index = 1);
            bool getMetaTable(const std::string& index);
            vsdk_ref* getThread(int index = 1);
            void* getUserData(int index = 1);
            int getReference(const std::string& name, bool pushValue = false);
            int getLength(int index = 1);

            // Pushers //
            void pushNil();
            void pushBool(bool value);
            void pushString(const std::string& value);
            void pushNumber(int value);
            void pushNumber(float value);
            void pushNumber(double value);
            void pushTable();
            void pushFunction(vsdk_exec& exec);

            // Registerers //
            void registerNil(const std::string& index);
            void registerNil(const std::string& index, const std::string& parent);
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
            void registerTable(const std::string& index);
            void registerTable(const std::string& index, const std::string& parent);
            void registerFunction(const std::string& index, vsdk_exec& exec);
            void registerFunction(const std::string& index, vsdk_exec& exec, const std::string& parent);
            void registerObject(const std::string& index, void* value);

            // Utils //
            void push(int index = 1);
            void pop(int count = 1);
            void move(create* target, int count = 1);
            bool pcall(int arguments, int returns);
            void removeReference(const std::string& name);
            void resume(int count = 0);
            void pause(int count = 0);
            void hook(const std::string& mode);
            int execute(std::function<int()> exec);
            bool loadString(const std::string& buffer, bool autoload = true);
            void throwError(const std::string& error = "");
    };
    typedef create vsdk_vm;
    typedef std::map<vsdk_ref*, vsdk_vm*> vsdk_vms;
    extern vsdk_vms fetchVMs();
    extern vsdk_vm* fetchVM(vsdk_ref* vm);
    extern vsdk_vm* toVM(void* vm);
}
