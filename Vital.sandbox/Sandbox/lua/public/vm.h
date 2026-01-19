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
#include <Vital.sandbox/vital.hpp>
#include <Vital.sandbox/Vendor/lua/lua.hpp>


//////////////////////////
// Vital: Sandbox: Lua //
//////////////////////////

namespace Vital::Sandbox::Lua {
    using vsdk_reference = std::map<std::string, int>;
    using vsdk_apis = std::vector<std::pair<std::function<void(void*)>, std::function<void(void*)>>>;


    // Globals //
    class create;
    inline create* toVM(void* vm) { return static_cast<create*>(vm); }

    namespace API {
        extern void createErrorHandle(std::function<void(const std::string&)> exec);
        extern void error(const std::string& error);
        extern void bind(create* vm, const std::string& parent, const std::string& name, lua_CFunction exec);
    }

    // Class //
    class create {
        protected:
            static inline std::vector<luaL_Reg> whitelist = {
                {"_G", luaopen_base},
                {"table", luaopen_table},
                {"string", luaopen_string},
                {"math", luaopen_math},
                {"debug", luaopen_debug},
                {"coroutine", luaopen_coroutine},
                {"utf8", luaopen_utf8},
                {"json", luaopen_rapidjson}
            };
            static inline std::vector<std::string> blacklist = {
                "dofile",
                "load",
                "loadfile"
            };
            static inline std::map<lua_State*, create*> buffer;
        private:
            lua_State* vm = nullptr;
            vsdk_reference reference = {};
            vsdk_apis apis = {};
            bool thread = false;
        public:
            // Instantiators //
            inline create(vsdk_apis apis = {}) {
                vm = luaL_newstate();
                this -> apis = apis;
                buffer.emplace(vm, this);
                for (auto& i : whitelist) {
                    luaL_requiref(vm, i.name, i.func, 1);
                    pop();
                }
                for (auto& i : blacklist) {
                    setNil();
                    setGlobal(i);
                }
                hook("bind");
                for (auto& i : Vital::Tool::get_modules("lua")) {
                    loadString(i);
                }
                hook("inject");
            }
            inline create(lua_State* thread) {
                vm = thread;
                this -> thread = true;
                buffer.emplace(vm, this);
            }
            inline ~create() {
                if (!vm) return;
                buffer.erase(vm);
                vm = nullptr;
            }
            static inline std::map<lua_State*, create*> fetchVMs() {
                return buffer;
            }
            static inline create* fetchVM(lua_State* vm) {
                auto it = buffer.find(vm);
                return it != buffer.end() ? it -> second : nullptr;
            }

        
            // Checkers //
            inline bool isVirtualThread() { return thread; }
            inline bool isNil(int index = 1) { return lua_isnoneornil(vm, index); }
            inline bool isBool(int index = 1) { return lua_isboolean(vm, index); }
            inline bool isString(int index = 1) { return lua_isstring(vm, index); }
            inline bool isNumber(int index = 1) { return lua_isnumber(vm, index); }
            inline bool isTable(int index = 1) { return lua_istable(vm, index); }
            inline bool isThread(int index = 1) { return lua_isthread(vm, index); }
            inline bool isUserData(int index = 1) { return lua_isuserdata(vm, index); }
            inline bool isFunction(int index = 1) { return lua_isfunction(vm, index); }
            inline bool isReference(const std::string& name) { return reference.find(name) != reference.end(); }


            // Setters //
            inline void setGlobal(const std::string& index) { lua_setglobal(vm, index.c_str()); }
            inline void setNil() { lua_pushnil(vm); }
            inline void setBool(bool value) { lua_pushboolean(vm, value); }
            inline void setString(const std::string& value) { lua_pushstring(vm, value.c_str()); }
            inline void setNumber(int value) { lua_pushnumber(vm, value); }
            inline void setNumber(float value) { lua_pushnumber(vm, value); }
            inline void setNumber(double value) { lua_pushnumber(vm, value); }
            inline void createTable() { lua_newtable(vm); }
            inline void setTable(int index = 1) { lua_settable(vm, index); }
            inline void setTableField(int value, int index = 1) { lua_seti(vm, index, value); }
            inline void setTableField(const std::string& value, int index = 1) { lua_setfield(vm, index, value.c_str()); }
            inline void createMetaTable(const std::string& value) { luaL_newmetatable(vm, value.c_str()); }
            inline void setMetaTable(int index = 1) { lua_setmetatable(vm, index);}
            inline void setMetaTable(const std::string& index) { luaL_setmetatable(vm, index.c_str()); }
            inline void createNamespace(const std::string& parent) {
                getGlobal(parent);
                if (!isTable(-1)) {
                    pop();
                    createTable();
                    setGlobal(parent);
                    getGlobal(parent);
                }
            }
            inline create* createThread() { return new create(lua_newthread(vm)); }
            inline void createUserData(void* value) {
                void** userdata = static_cast<void**>(lua_newuserdata(vm, sizeof(void*)));
                *userdata = value;
            }
            inline void setUserData(void* value) { lua_pushlightuserdata(vm, value); }
            inline void setFunction(lua_CFunction& value) { lua_pushcfunction(vm, value); }
            inline void setReference(const std::string& name, int index = 1) {
                push(index);
                reference.emplace(name, luaL_ref(vm, LUA_REGISTRYINDEX));
            }


            // Getters //
            inline int getArgCount() { return lua_gettop(vm); }
            inline bool getGlobal(const std::string& index) { return lua_getglobal(vm, index.c_str()); }
            inline bool getBool(int index = 1) { return lua_toboolean(vm, index); }
            inline std::string getString(int index = 1) { return lua_tostring(vm, index); }
            inline int getInt(int index = 1) { return (int)lua_tonumber(vm, index); }
            inline float getFloat(int index = 1) { return (float)lua_tonumber(vm, index); }
            inline double getDouble(int index = 1) { return lua_tonumber(vm, index); }
            inline bool getTable(int index = 1) { return lua_gettable(vm, index); }
            inline bool getTableField(int value, int index = 1) { return lua_geti(vm, index, value); }
            inline bool getTableField(const std::string& value, int index = 1) {return lua_getfield(vm, index, value.c_str());}
            inline bool getMetaTable(int index = 1) { return lua_getmetatable(vm, index); }
            inline bool getMetaTable(const std::string& index) { return luaL_getmetatable(vm, index.c_str()); }
            inline lua_State* getThread(int index = 1) { return lua_tothread(vm, index); }
            inline void* getUserData(int index = 1) { return lua_touserdata(vm, index); }
            inline int getReference(const std::string& name, bool pushValue = false) {
                if (!pushValue) return reference.at(name);
                lua_rawgeti(vm, LUA_REGISTRYINDEX, reference.at(name));
                return 0;
            }
            inline int getLength(int index = 1) {
                lua_len(vm, index);
                int result = getInt(-1);
                pop();
                return result;
            }


            // Pushers //
            inline void pushNil() {
                setNil();
                setTableField(getLength(-2) + 1, -2);
            }
            inline void pushBool(bool value) {
                setBool(value);
                setTableField(getLength(-2) + 1, -2);
            }
            inline void pushString(const std::string& value) {
                setString(value);
                setTableField(getLength(-2) + 1, -2);
            }
            inline void pushNumber(int value) {
                setNumber(value);
                setTableField(getLength(-2) + 1, -2);
            }
            inline void pushNumber(float value) {
                setNumber(value);
                setTableField(getLength(-2) + 1, -2);
            }
            inline void pushNumber(double value) {
                setNumber(value);
                setTableField(getLength(-2) + 1, -2);
            }
            inline void pushTable() {
                if (!isTable(-1)) return;
                setTableField(getLength(-2) + 1, -2);
            }
            inline void pushFunction(lua_CFunction& exec) {
                setFunction(exec);
                setTableField(getLength(-2) + 1, -2);
            }

        
            // Registerers //
            inline void registerNil(const std::string& index) {
                setNil();
                setTableField(index, -2);
            }
            inline void registerNil(const std::string& index, const std::string& parent) {
                createNamespace(parent);
                registerNil(index);
                pop();
            }
            inline void registerBool(const std::string& index, bool value) {
                setBool(value);
                setTableField(index, -2);
            }
            inline void registerBool(const std::string& index, bool value, const std::string& parent) {
                createNamespace(parent);
                registerBool(index, value);
                pop();
            }
            inline void registerString(const std::string& index, const std::string& value) {
                setString(value);
                setTableField(index, -2);
            }
            inline void registerString(const std::string& index, const std::string& value, const std::string& parent) {
                createNamespace(parent);
                registerString(index, value);
                pop();
            }
            inline void registerNumber(const std::string& index, int value) {
                setNumber(value);
                setTableField(index, -2);
            }
            inline void registerNumber(const std::string& index, int value, const std::string& parent) {
                createNamespace(parent);
                registerNumber(index, value);
                pop();
            }
            inline void registerNumber(const std::string& index, float value) {
                setNumber(value);
                setTableField(index, -2);
            }
            inline void registerNumber(const std::string& index, float value, const std::string& parent) {
                createNamespace(parent);
                registerNumber(index, value);
                pop();
            }
            inline void registerNumber(const std::string& index, double value) {
                setNumber(value);
                setTableField(index, -2);
            }
            inline void registerNumber(const std::string& index, double value, const std::string& parent) {
                createNamespace(parent);
                registerNumber(index, value);
                pop();
            }
            inline void registerTable(const std::string& index) {
                if (!isTable(-1)) return;
                setTableField(index, -2);
            }
            inline void registerTable(const std::string& index, const std::string& parent) {
                if (!isTable(-1)) return;
                createNamespace(parent);
                registerTable(index);
                pop();
            }
            inline void registerFunction(const std::string& index, lua_CFunction& exec) {
                setFunction(exec);
                setTableField(index, -2);
            }
            inline void registerFunction(const std::string& index, lua_CFunction& exec, const std::string& parent) {
                createNamespace(parent);
                registerFunction(index, exec);
                pop();
            }
            inline void registerObject(const std::string& index, void* value) {
                createUserData(value);
                setMetaTable(index);
            }


            // Utils //
            inline void push(int index = 1) { lua_pushvalue(vm, index); }
            inline void pop(int count = 1) { lua_pop(vm, count); }
            inline void move(create* target, int count = 1) { lua_xmove(vm, target -> vm, count); }
            inline bool pcall(int arguments, int returns) { return lua_pcall(vm, arguments, returns, 0); }
            inline void removeReference(const std::string& name) {
                if (!isReference(name)) return;
                luaL_unref(vm, LUA_REGISTRYINDEX, getReference(name));
                reference.erase(name);
            }
            inline void resume(int count = 0) {
                if (!isVirtualThread()) return;
                int ncount;
                lua_resume(vm, nullptr, count, &ncount);
                if (lua_status(vm) != LUA_YIELD) delete this;
            }
            inline void pause(int count = 0) {
                if (!isVirtualThread()) return;
                lua_yield(vm, count);
            }
            inline int execute(std::function<int()> exec) {
                try { return exec(); }
                catch(const std::runtime_error& error) { throwError(error.what()); }
                catch(...) { throwError(); }
                return 1;
            }
            inline bool loadString(const std::string& buf, bool autoload = true) {
                if (buf.empty()) return false;
                if (!autoload) {
                    std::string b = "return (function() " + buf + " end)";
                    luaL_loadstring(vm, b.c_str());
                }
                else luaL_loadstring(vm, buf.c_str());
                if (pcall(0, LUA_MULTRET)) {
                    API::error(getString(-1));
                    pop();
                    return false;
                }
                return true;
            }
            inline void throwError(const std::string& error = "") {
                lua_Debug debug;
                lua_getstack(vm, 1, &debug);
                lua_getinfo(vm, "nSl", &debug);
                API::error("[ERROR - L" + std::to_string(debug.currentline) + "] | Reason: " + (error.empty() ? "N/A" : error));
                setBool(false);
            }


            void hook(const std::string& mode);

    };
}