/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: machine.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Machine Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/index.h>


//////////////////////////////
// Vital: Sandbox: Machine //
//////////////////////////////

namespace Vital::Sandbox {
    class Machine {
        protected:
            static vm_apis natives;
            inline static vm_buffer buffer;
            inline static std::vector<luaL_Reg> whitelist = {
                {"_G", luaopen_base},
                {"table", luaopen_table},
                {"string", luaopen_string},
                {"math", luaopen_math},
                {"debug", luaopen_debug},
                {"coroutine", luaopen_coroutine},
                {"utf8", luaopen_utf8},
                {"json", luaopen_rapidjson}
            };
            inline static std::vector<std::string> blacklist = {
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
            Machine(vm_apis apis = {}) : apis(std::move(apis)) {
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
                for (auto& value : Vital::Tool::fetch_modules("lua")) {
                    load_string(value);
                }
                hook("inject");
            }

            Machine(vm_state* thread) {
                vm = thread;
                virtualized = true;
                buffer.emplace(vm, this);
            }

            ~Machine() {
                if (!vm) return;
                if (!virtualized) lua_close(vm);
                buffer.erase(vm);
                vm = nullptr;
            }


            // APIs //
            static const vm_buffer fetch_buffer() { return buffer; }
            static Machine* to_machine(void* vm) { return static_cast<Machine*>(vm); }
            static void* to_void(Machine* vm) { return static_cast<void*>(vm); }
            vm_state* fetch_state() { return vm; }
            static Machine* fetch_machine(vm_state* vm) {
                auto it = buffer.find(vm);
                return it != buffer.end() ? it -> second : nullptr;
            }


            // Checkers //
            bool is_virtual() { return virtualized; }
            bool is_nil(int index = 1) { return lua_isnoneornil(vm, index); }
            bool is_bool(int index = 1) { return lua_isboolean(vm, index); }
            bool is_string(int index = 1) { return lua_isstring(vm, index); }
            bool is_number(int index = 1) { return lua_isnumber(vm, index); }
            bool is_table(int index = 1) { return lua_istable(vm, index); }
            bool is_thread(int index = 1) { return lua_isthread(vm, index); }
            bool is_userdata(int index = 1) { return lua_isuserdata(vm, index); }
            bool is_function(int index = 1) { return lua_isfunction(vm, index); }
            bool is_reference(const std::string& name) { return reference.find(name) != reference.end(); }


            // Getters //
            int get_arg_count() { return lua_gettop(vm); }
            bool get_global(const std::string& index) { return lua_getglobal(vm, index.c_str()); }
            bool get_bool(int index = 1) { return lua_toboolean(vm, index); }
            std::string get_string(int index = 1) { return lua_tostring(vm, index); }
            int get_int(int index = 1) { return (int)lua_tonumber(vm, index); }
            float get_float(int index = 1) { return (float)lua_tonumber(vm, index); }
            double get_double(int index = 1) { return lua_tonumber(vm, index); }
            bool get_table(int index = 1) { return lua_gettable(vm, index); }
            bool get_table_field(int value, int index = 1) { return lua_geti(vm, index, value); }
            bool get_table_field(const std::string& value, int index = 1) {return lua_getfield(vm, index, value.c_str());}
            bool get_metatable(int index = 1) { return lua_getmetatable(vm, index); }
            bool get_metatable(const std::string& index) { return luaL_getmetatable(vm, index.c_str()); }
            vm_state* get_thread(int index = 1) { return lua_tothread(vm, index); }
            void* get_userdata(int index = 1) { return lua_touserdata(vm, index); }
            int get_reference(const std::string& name, bool push_to_stack = false) {
                if (!push_to_stack) return reference.at(name);
                lua_rawgeti(vm, LUA_REGISTRYINDEX, reference.at(name));
                return 0;
            }
            int get_length(int index = 1) {
                lua_len(vm, index);
                int result = get_int(-1);
                pop();
                return result;
            }
        
        
            // Pushers //
            void push_global(const std::string& index) { lua_setglobal(vm, index.c_str()); }
            void push_nil() { lua_pushnil(vm); }
            void push_bool(bool value) { lua_pushboolean(vm, value); }
            void push_string(const std::string& value) { lua_pushstring(vm, value.c_str()); }
            void push_number(int value) { lua_pushinteger(vm, value); }
            void push_number(float value) { lua_pushnumber(vm, value); }
            void push_number(double value) { lua_pushnumber(vm, value); }
            void push_userdata(void* value) { lua_pushlightuserdata(vm, value); }
            void push_function(const vm_exec& value) { lua_pushcfunction(vm, value); }
            void push_reference(const std::string& name, int index = 1) {
                push(index);
                reference.emplace(name, luaL_ref(vm, LUA_REGISTRYINDEX));
            }
        

            // Containers //
            void create_table() { lua_newtable(vm); }
            void create_metatable(const std::string& value) { luaL_newmetatable(vm, value.c_str()); }
            Machine* create_thread() { return new Machine(lua_newthread(vm)); }
            void create_namespace(const std::string& nspace) {
                get_global(nspace);
                if (!is_table(-1)) {
                    pop();
                    create_table();
                    push_global(nspace);
                    get_global(nspace);
                }
            }
            void create_userdata(void* value) {
                void** userdata = static_cast<void**>(lua_newuserdata(vm, sizeof(void*)));
                *userdata = value;
            }
            void create_object(const std::string& index, void* value) {
                create_userdata(value);
                set_metatable(index);
            }


            // Container Pushers //
            void table_push_nil(const std::string& nspace = "") {
                if (!nspace.empty()) create_namespace(nspace);
                push_nil();
                set_table_field(get_length(-2) + 1, -2);
                if (!nspace.empty()) pop();
            }
            void table_push_bool(bool value, const std::string& nspace = "") {
                if (!nspace.empty()) create_namespace(nspace);
                push_bool(value);
                set_table_field(get_length(-2) + 1, -2);
                if (!nspace.empty()) pop();
            }
            void table_push_string(const std::string& value, const std::string& nspace = "") {
                if (!nspace.empty()) create_namespace(nspace);
                push_string(value);
                set_table_field(get_length(-2) + 1, -2);
                if (!nspace.empty()) pop();
            }
            void table_push_number(int value, const std::string& nspace = "") {
                if (!nspace.empty()) create_namespace(nspace);
                push_number(value);
                set_table_field(get_length(-2) + 1, -2);
                if (!nspace.empty()) pop();
            }
            void table_push_number(float value, const std::string& nspace = "") {
                if (!nspace.empty()) create_namespace(nspace);
                push_number(value);
                set_table_field(get_length(-2) + 1, -2);
                if (!nspace.empty()) pop();
            }
            void table_push_number(double value, const std::string& nspace = "") {
                if (!nspace.empty()) create_namespace(nspace);
                push_number(value);
                set_table_field(get_length(-2) + 1, -2);
                if (!nspace.empty()) pop();
            }
            void table_push_table(const std::string& nspace = "") {
                if (!nspace.empty()) {
                    create_namespace(nspace);
                    get_table(-2);
                }
                set_table_field(get_length(-2) + 1, -2);
                if (!nspace.empty()) pop(2);
            }
            void table_push_function(vm_exec& exec, const std::string& nspace = "") {
                if (!nspace.empty()) create_namespace(nspace);
                push_function(exec);
                set_table_field(get_length(-2) + 1, -2);
                if (!nspace.empty()) pop();
            }
            void table_set_nil(const std::string& index, const std::string& nspace = "") {
                if (!nspace.empty()) create_namespace(nspace);
                push_nil();
                set_table_field(index, -2);
                if (!nspace.empty()) pop();
            }
            void table_set_bool(const std::string& index, bool value, const std::string& nspace = "") {
                if (!nspace.empty()) create_namespace(nspace);
                push_bool(value);
                set_table_field(index, -2);
                if (!nspace.empty()) pop();
            }
            void table_set_string(const std::string& index, const std::string& value, const std::string& nspace = "") {
                if (!nspace.empty()) create_namespace(nspace);
                push_string(value);
                set_table_field(index, -2);
                if (!nspace.empty()) pop();
            }
            void table_set_number(const std::string& index, int value, const std::string& nspace = "") {
                if (!nspace.empty()) create_namespace(nspace);
                push_number(value);
                set_table_field(index, -2);
                if (!nspace.empty()) pop();
            }
            void table_set_number(const std::string& index, float value, const std::string& nspace = "") {
                if (!nspace.empty()) create_namespace(nspace);
                push_number(value);
                set_table_field(index, -2);
                if (!nspace.empty()) pop();
            }
            void table_set_number(const std::string& index, double value, const std::string& nspace = "") {
                if (!nspace.empty()) create_namespace(nspace);
                push_number(value);
                set_table_field(index, -2);
                if (!nspace.empty()) pop();
            }
            void table_set_table(const std::string& index, const std::string& nspace = "") {
                if (!nspace.empty()) {
                    create_namespace(nspace);
                    get_table(-2);
                }
                set_table_field(index, -2);
                if (!nspace.empty()) pop(2);
            }
            void table_set_function(const std::string& index, vm_exec& exec, const std::string& nspace = "") {
                if (!nspace.empty()) create_namespace(nspace);
                push_function(exec);
                set_table_field(index, -2);
                if (!nspace.empty()) pop();
            }


            // Loggers //
            void log(const std::string& type, const std::string& message = "") {
                lua_Debug debug;
                lua_getstack(vm, 1, &debug);
                lua_getinfo(vm, "nSl", &debug);
                API::log(type, fmt::format("{} {}", message.empty() ? "N/A" : message, "[Line: " + std::to_string(debug.currentline) + "]"));
                push_bool(false);
            }
    
            template<typename F>
            int execute(F&& exec) {
                try { return exec(); }
                catch (const Vital::Log::Info& e) { log(std::string(Vital::Log::Info::label), e.what()); }
                catch (const Vital::Log::Warning& e) { log(std::string(Vital::Log::Warning::label), e.what()); }
                catch (const Vital::Log::Error& e) { log(std::string(Vital::Log::Error::label), e.what()); }
                catch (const std::runtime_error& e) { log(std::string(Vital::Log::Error::label), e.what()); }
                catch (...) { log(std::string(Vital::Log::Error::label)); }
                return 1;
            }


            // Utils //
            void push(int index = 1) { lua_pushvalue(vm, index); }
            void pop(int count = 1) { lua_pop(vm, count); }
            void move(Machine* target, int count = 1) { lua_xmove(vm, target -> vm, count); }
            bool pcall(int arguments, int returns) { return lua_pcall(vm, arguments, returns, 0) == LUA_OK; }
            void set_table(int index = 1) { lua_settable(vm, index); }
            void set_table_field(int field, int index = 1) { lua_seti(vm, index, field); }
            void set_table_field(const std::string& field, int index = 1) { lua_setfield(vm, index, field.c_str()); }
            void set_metatable(int index = 1) { lua_setmetatable(vm, index);}
            void set_metatable(const std::string& index) { luaL_setmetatable(vm, index.c_str()); }

            void remove_reference(const std::string& name) {
                if (!is_reference(name)) return;
                luaL_unref(vm, LUA_REGISTRYINDEX, get_reference(name));
                reference.erase(name);
            }

            void resume(int count = 0) {
                if (!is_virtual()) return;
                int ncount;
                lua_resume(vm, nullptr, count, &ncount);
                if (lua_status(vm) != LUA_YIELD) delete this;
            }

            void pause(int count = 0) {
                if (!is_virtual()) return;
                lua_yield(vm, count);
            }
        
            std::string to_string(int index = 1) {
                size_t length;
                const char* value = luaL_tolstring(vm, index, &length);
                pop(1);
                return std::string(value, length);
            }

            bool load_string(const std::string& raw, bool autoload = true) {
                if (raw.empty()) return false;
                if (luaL_loadstring(vm, raw.c_str()) != LUA_OK) {
                    API::log(std::string(Vital::Log::Error::label), get_string(-1));
                    pop();
                    return false;
                }
                if (autoload) {
                    if (!pcall(0, LUA_MULTRET)) {
                        API::log(std::string(Vital::Log::Error::label), get_string(-1));
                        pop();
                        return false;
                    }
                }                
                return true;
            }
        
            void hook(const std::string& mode) {
                auto vm_ptr = to_void(this);
                for (auto& i : natives) {
                    mode == "bind" ? i.first(vm_ptr) : i.second(vm_ptr);
                }
                for (auto& i : apis) {
                    mode == "bind" ? i.first(vm_ptr) : i.second(vm_ptr);
                }
            }

            // TODO: BETTER WAY TO BIND FUNCTION
            void bind_function(const std::string& nspace, const std::string& name, vm_bind fn) {
                auto* heap_fn = new vm_bind(std::move(fn));
                create_namespace(nspace);
                lua_pushlightuserdata(vm, heap_fn);
                lua_pushcclosure(vm, [](lua_State* ref) -> int {
                    auto* fn = static_cast<vm_bind*>(lua_touserdata(ref, lua_upvalueindex(1)));
                    auto vm = Machine::fetch_machine(ref);
                    return vm -> execute([&]() -> int {
                        return (*fn)(vm);
                    });
                }, 1);
                set_table_field(name, -2);
                pop();
            }
    };
}