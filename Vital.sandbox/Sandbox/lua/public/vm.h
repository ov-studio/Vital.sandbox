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
    class create;
    using vm_state = lua_State;
    using vm_exec = lua_CFunction;
    using vm_buffer = std::map<vm_state*, create*>;
    using vm_refs = std::map<std::string, int>;
    using vm_apis = std::vector<std::pair<std::function<void(void*)>, std::function<void(void*)>>>;


    // Globals //
    namespace API {
        extern void createErrorHandle(std::function<void(const std::string&)> exec);
        extern void error(const std::string& error);
        extern void bind(create* vm, const std::string& namespace, const std::string& name, vm_exec exec);
    }

    class create {
        protected:
            static inline vm_buffer buffer;
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
        private:
            bool virtualized = false;
            vm_state* vm = nullptr;
            vm_refs reference = {};
            vm_apis apis = {};
        public:
            inline create(vm_apis apis = {}) {
                vm = luaL_newstate();
                this -> apis = apis;
                buffer.emplace(vm, this);
                for (auto& value : whitelist) {
                    luaL_requiref(vm, value.name, value.func, 1);
                    pop();
                }
                for (auto& value : blacklist) {
                    push_nil();
                    push_global(value);
                }
                hook("bind");
                for (auto& value : Vital::Tool::get_modules("lua")) {
                    load_string(value);
                }
                hook("inject");
            }

            inline create(vm_state* thread) {
                vm = thread;
                virtualized = true;
                buffer.emplace(vm, this);
            }

            inline ~create() {
                if (!vm) return;
                buffer.erase(vm);
                vm = nullptr;
            }


            // Checkers //
            inline bool is_virtual() { return virtualized; }
            inline bool is_nil(int index = 1) { return lua_isnoneornil(vm, index); }
            inline bool is_bool(int index = 1) { return lua_isboolean(vm, index); }
            inline bool is_string(int index = 1) { return lua_isstring(vm, index); }
            inline bool is_number(int index = 1) { return lua_isnumber(vm, index); }
            inline bool is_table(int index = 1) { return lua_istable(vm, index); }
            inline bool is_thread(int index = 1) { return lua_isthread(vm, index); }
            inline bool is_userdata(int index = 1) { return lua_isuserdata(vm, index); }
            inline bool is_function(int index = 1) { return lua_isfunction(vm, index); }
            inline bool is_reference(const std::string& name) { return reference.find(name) != reference.end(); }


            // Pushers //
            inline void push_global(const std::string& index) { lua_setglobal(vm, index.c_str()); }
            inline void push_nil() { lua_pushnil(vm); }
            inline void push_bool(bool value) { lua_pushboolean(vm, value); }
            inline void push_string(const std::string& value) { lua_pushstring(vm, value.c_str()); }
            inline void push_number(int value) { lua_pushnumber(vm, value); }
            inline void push_number(float value) { lua_pushnumber(vm, value); }
            inline void push_number(double value) { lua_pushnumber(vm, value); }
            inline void push_table(int index = 1) { lua_settable(vm, index); }
            inline void push_table_field(int value, int index = 1) { lua_seti(vm, index, value); }
            inline void push_table_field(const std::string& value, int index = 1) { lua_setfield(vm, index, value.c_str()); }
            inline void push_meta_table(int index = 1) { lua_setmetatable(vm, index);}
            inline void push_meta_table(const std::string& index) { luaL_setmetatable(vm, index.c_str()); }
            inline void push_userdata(void* value) { lua_pushlightuserdata(vm, value); }
            inline void push_function(vm_exec& value) { lua_pushcfunction(vm, value); }
            inline void push_reference(const std::string& name, int index = 1) {
                push(index);
                reference.emplace(name, luaL_ref(vm, LUA_REGISTRYINDEX));
            }
        

            // Getters //
            inline int get_arg_count() { return lua_gettop(vm); }
            inline bool get_global(const std::string& index) { return lua_getglobal(vm, index.c_str()); }
            inline bool get_bool(int index = 1) { return lua_toboolean(vm, index); }
            inline std::string get_string(int index = 1) { return lua_tostring(vm, index); }
            inline int get_int(int index = 1) { return (int)lua_tonumber(vm, index); }
            inline float get_float(int index = 1) { return (float)lua_tonumber(vm, index); }
            inline double get_double(int index = 1) { return lua_tonumber(vm, index); }
            inline bool get_table(int index = 1) { return lua_gettable(vm, index); }
            inline bool get_table_field(int value, int index = 1) { return lua_geti(vm, index, value); }
            inline bool get_table_field(const std::string& value, int index = 1) {return lua_getfield(vm, index, value.c_str());}
            inline bool get_meta_table(int index = 1) { return lua_getmetatable(vm, index); }
            inline bool get_meta_table(const std::string& index) { return luaL_getmetatable(vm, index.c_str()); }
            inline vm_state* get_thread(int index = 1) { return lua_tothread(vm, index); }
            inline void* get_userdata(int index = 1) { return lua_touserdata(vm, index); }
            inline int get_reference(const std::string& name, bool pushValue = false) {
                if (!pushValue) return reference.at(name);
                lua_rawgeti(vm, LUA_REGISTRYINDEX, reference.at(name));
                return 0;
            }
            inline int get_length(int index = 1) {
                lua_len(vm, index);
                int result = get_int(-1);
                pop();
                return result;
            }
        

            // Containers //
            inline void create_table() { lua_newtable(vm); }
            inline void create_meta_table(const std::string& value) { luaL_newmetatable(vm, value.c_str()); }
            inline create* create_thread() { return new create(lua_newthread(vm)); }
            inline void create_namespace(const std::string& namespace) {
                get_global(namespace);
                if (!is_table(-1)) {
                    pop();
                    create_table();
                    push_global(namespace);
                    get_global(namespace);
                }
            }
            inline void create_userdata(void* value) {
                void** userdata = static_cast<void**>(lua_newuserdata(vm, sizeof(void*)));
                *userdata = value;
            }
            inline void create_object(const std::string& index, void* value) {
                create_userdata(value);
                push_meta_table(index);
            }


            // Pushers //
            inline void table_push_nil() {
                push_nil();
                push_table_field(get_length(-2) + 1, -2);
            }
            inline void table_push_bool(bool value) {
                push_bool(value);
                push_table_field(get_length(-2) + 1, -2);
            }
            inline void table_push_string(const std::string& value) {
                push_string(value);
                push_table_field(get_length(-2) + 1, -2);
            }
            inline void table_push_number(int value) {
                push_number(value);
                push_table_field(get_length(-2) + 1, -2);
            }
            inline void table_push_number(float value) {
                push_number(value);
                push_table_field(get_length(-2) + 1, -2);
            }
            inline void table_push_number(double value) {
                push_number(value);
                push_table_field(get_length(-2) + 1, -2);
            }
            inline void table_push_table() {
                if (!is_table(-1)) return;
                push_table_field(get_length(-2) + 1, -2);
            }
            inline void table_push_function(vm_exec& exec) {
                push_function(exec);
                push_table_field(get_length(-2) + 1, -2);
            }

            inline void table_set_nil(const std::string& index, const std::string& namespace = "") {
                if (!namespace.empty()) create_namespace(namespace);
                push_nil();
                push_table_field(index, -2);
                if (!namespace.empty()) pop();
            }
            inline void table_set_bool(const std::string& index, bool value, const std::string& namespace = "") {
                if (!namespace.empty()) create_namespace(namespace);
                push_bool(value);
                push_table_field(index, -2);
                if (!namespace.empty()) pop();
            }
            inline void table_set_string(const std::string& index, const std::string& value, const std::string& namespace = "") {
                if (!namespace.empty()) create_namespace(namespace);
                push_string(value);
                push_table_field(index, -2);
                if (!namespace.empty()) pop();
            }
            inline void table_set_number(const std::string& index, int value, const std::string& namespace = "") {
                if (!namespace.empty()) create_namespace(namespace);
                push_number(value);
                push_table_field(index, -2);
                if (!namespace.empty()) pop();
            }
            inline void table_set_number(const std::string& index, float value, const std::string& namespace = "") {
                if (!namespace.empty()) create_namespace(namespace);
                push_number(value);
                push_table_field(index, -2);
                if (!namespace.empty()) pop();
            }
            inline void table_set_number(const std::string& index, double value, const std::string& namespace = "") {
                if (!namespace.empty()) create_namespace(namespace);
                push_number(value);
                push_table_field(index, -2);
                if (!namespace.empty()) pop();
            }
            inline void table_set_table(const std::string& index, const std::string& namespace = "") {
                //if (!is_table(-1)) return;
                if (!namespace.empty()) create_namespace(namespace);
                push_table_field(index, -2);
                if (!namespace.empty()) pop();
            }
            inline void table_set_function(const std::string& index, vm_exec& exec, const std::string& namespace = "") {
                if (!namespace.empty()) create_namespace(namespace);
                push_function(exec);
                push_table_field(index, -2);
                if (!namespace.empty()) pop();
            }


            // APIs //
            static inline vm_buffer fetch_buffer() { return buffer; }
            static inline create* to_vm(void* vm) { return static_cast<create*>(vm); }
            static inline create* fetch_vm(vm_state* vm) {
                auto it = buffer.find(vm);
                return it != buffer.end() ? it -> second : nullptr;
            }


            // Utils //
            inline void push(int index = 1) { lua_pushvalue(vm, index); }
            inline void pop(int count = 1) { lua_pop(vm, count); }
            inline void move(create* target, int count = 1) { lua_xmove(vm, target -> vm, count); }
            inline bool pcall(int arguments, int returns) { return lua_pcall(vm, arguments, returns, 0); }
            inline void removeReference(const std::string& name) {
                if (!is_reference(name)) return;
                luaL_unref(vm, LUA_REGISTRYINDEX, get_reference(name));
                reference.erase(name);
            }
            inline void resume(int count = 0) {
                if (!is_virtual()) return;
                int ncount;
                lua_resume(vm, nullptr, count, &ncount);
                if (lua_status(vm) != LUA_YIELD) delete this;
            }
            inline void pause(int count = 0) {
                if (!is_virtual()) return;
                lua_yield(vm, count);
            }
            inline int execute(std::function<int()> exec) {
                try { return exec(); }
                catch(const std::runtime_error& error) { throw_error(error.what()); }
                catch(...) { throw_error(); }
                return 1;
            }
            inline bool load_string(const std::string& buf, bool autoload = true) {
                if (buf.empty()) return false;
                if (!autoload) {
                    std::string b = "return (function() " + buf + " end)";
                    luaL_loadstring(vm, b.c_str());
                }
                else luaL_loadstring(vm, buf.c_str());
                if (pcall(0, LUA_MULTRET)) {
                    API::error(get_string(-1));
                    pop();
                    return false;
                }
                return true;
            }
            inline void throw_error(const std::string& error = "") {
                lua_Debug debug;
                lua_getstack(vm, 1, &debug);
                lua_getinfo(vm, "nSl", &debug);
                API::error("[ERROR - L" + std::to_string(debug.currentline) + "] | Reason: " + (error.empty() ? "N/A" : error));
                push_bool(false);
            }

            void hook(const std::string& mode);

    };
}