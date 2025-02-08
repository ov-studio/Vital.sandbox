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


//////////////
// Lua: VM //
//////////////

namespace Vital::Sandbox::Lua {
    typedef lua_State vital_ref;
    typedef lua_CFunction vital_exec;
    typedef std::pair<std::string, std::string> vital_bind;
    class create {
        private:
            vital_ref* vm = nullptr;
            bool thread = false;
        public:
            // Instantiators //
            create();
            create(vital_ref* thread);
            ~create();
            bool isVirtualThread();

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
            void setFunction(vital_exec& value);

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
            vital_ref* getThread(int index = 1);
            void* getUserData(int index = 1);
            int getLength(int index = 1);

            // Pushers //
            void pushBool(bool value);
            void pushString(const std::string& value);
            void pushNumber(int value);
            void pushNumber(float value);
            void pushNumber(double value);
            void pushTable();
            void pushFunction(vital_exec& exec);

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
            void registerTable(const std::string& index);
            void registerTable(const std::string& index, const std::string& parent);
            void registerFunction(const std::string& index, vital_exec& exec);
            void registerFunction(const std::string& index, vital_exec& exec, const std::string& parent);
            void registerObject(const std::string& index, void* value);

            // Utils //
            void push(int index = 1);
            void pop(int count = 1);
            void move(create* target, int count = 1);
            void resume();
            void pause();
            int execute(std::function<int()> exec);
            bool loadString(const std::string& buffer, bool isAutoLoad = true);
            bool throwError(const std::string& error = "");
    };
    typedef create vital_vm;
    extern vital_vm* fetchVM(vital_ref* vm);
}
