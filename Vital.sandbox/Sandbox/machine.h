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
#include <Vital.sandbox/Sandbox/mixin.h>


//////////////////////////////
// Vital: Sandbox: Machine //
//////////////////////////////

namespace Vital::Sandbox {
    class Machine : public MachineMixin<Machine> {
        protected:
            static vm_apis internal_apis;
            inline static vm_machines machines;
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
            vm_state* state = nullptr;
            vm_refs reference = {};
            vm_apis external_apis = {};
        public:
            Machine(vm_apis apis = {}) : external_apis(std::move(apis)) {
                state = luaL_newstate();
                machines.emplace(state, this);
                for (auto& value : whitelist) {
                    luaL_requiref(state, value.name, value.func, 1);
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
                state = thread;
                virtualized = true;
                machines.emplace(state, this);
            }

            ~Machine() {
                if (!state) return;
                if (!virtualized) lua_close(state);
                machines.erase(state);
                state = nullptr;
            }


            // APIs //
            static Machine* to_machine(void* vm) { return static_cast<Machine*>(vm); }
            static const vm_machines fetch_machines() { return machines; }
            static Machine* fetch_machine(vm_state* state) {
                auto it = machines.find(state);
                return it != machines.end() ? it -> second : nullptr;
            }


            // Checkers //
            bool is_virtual() { return virtualized; }
            bool is_nil(int index = 1) { return lua_isnoneornil(state, index); }
            bool is_bool(int index = 1) { return lua_isboolean(state, index); }
            bool is_string(int index = 1) { return lua_isstring(state, index); }
            bool is_number(int index = 1) { return lua_isnumber(state, index); }
            bool is_table(int index = 1) { return lua_istable(state, index); }
            bool is_thread(int index = 1) { return lua_isthread(state, index); }
            bool is_userdata(int index = 1) { return lua_isuserdata(state, index); }
            bool is_function(int index = 1) { return lua_isfunction(state, index); }
            bool is_reference(const std::string& name) { return reference.find(name) != reference.end(); }


            // Getters //
            vm_state* get_state() { return state; }
            int get_arg_count() { return lua_gettop(state); }
            bool get_global(const std::string& index) { return lua_getglobal(state, index.c_str()); }
            bool get_bool(int index = 1) { return lua_toboolean(state, index); }
            std::string get_string(int index = 1) { return lua_tostring(state, index); }
            int get_int(int index = 1) { return (int)lua_tonumber(state, index); }
            float get_float(int index = 1) { return (float)lua_tonumber(state, index); }
            double get_double(int index = 1) { return lua_tonumber(state, index); }
            bool get_table(int index = 1) { return lua_gettable(state, index); }
            bool get_table_field(int value, int index = 1) { return lua_geti(state, index, value); }
            bool get_table_field(const std::string& value, int index = 1) {return lua_getfield(state, index, value.c_str());}
            bool get_metatable(int index = 1) { return lua_getmetatable(state, index); }
            bool get_metatable(const std::string& index) { return luaL_getmetatable(state, index.c_str()); }
            vm_state* get_thread(int index = 1) { return lua_tothread(state, index); }
            void* get_userdata(int index = 1) { return lua_touserdata(state, index); }
            int get_length(int index = 1) {
                lua_len(state, index);
                int result = get_int(-1);
                pop();
                return result;
            }
            int get_reference(const std::string& name, bool push_to_stack = false) {
                if (!push_to_stack) return reference.at(name);
                lua_rawgeti(state, LUA_REGISTRYINDEX, reference.at(name));
                return 0;
            }
            godot::Color get_color(int index = 1) {
                godot::Color value = {1, 1, 1, 1};
                get_table_field(1, index); value.r = get_float(-1); pop();
                get_table_field(2, index); value.g = get_float(-1); pop();
                get_table_field(3, index); value.b = get_float(-1); pop();
                get_table_field(4, index); value.a = get_float(-1); pop();
                return value;
            }
            godot::Vector2 get_vector2(int index = 1) {
                godot::Vector2 value = {0.0f, 0.0f};
                get_table_field(1, index); value.x = get_float(-1); pop();
                get_table_field(2, index); value.y = get_float(-1); pop();
                return value;
            }
            godot::PackedVector2Array get_vector2_array(int index = 1) {
                godot::PackedVector2Array value;
                int len = get_length(index);
                for (int i = 1; i <= len; ++i) {
                    get_table_field(i, index);
                    value.push_back(get_vector2(-1));
                    pop();
                }
                return value;
            }
            godot::Vector3 get_vector3(int index = 1) {
                godot::Vector3 value = {0.0f, 0.0f, 0.0f};
                get_table_field(1, index); value.x = get_float(-1); pop();
                get_table_field(2, index); value.y = get_float(-1); pop();
                get_table_field(3, index); value.z = get_float(-1); pop();
                return value;
            }
            godot::PackedVector3Array get_vector3_array(int index = 1) {
                godot::PackedVector3Array value;
                int len = get_length(index);
                for (int i = 1; i <= len; ++i) {
                    get_table_field(i, index);
                    value.push_back(get_vector3(-1));
                    pop();
                }
                return value;
            }


            // Pushers //
            void push_global(const std::string& index) { lua_setglobal(state, index.c_str()); }
            void push_nil() { lua_pushnil(state); }
            void push_bool(bool value) { lua_pushboolean(state, value); }
            void push_string(const std::string& value) { lua_pushstring(state, value.c_str()); }
            void push_number(int value) { lua_pushinteger(state, value); }
            void push_number(float value) { lua_pushnumber(state, value); }
            void push_number(double value) { lua_pushnumber(state, value); }
            void push_userdata(void* value) { lua_pushlightuserdata(state, value); }
            void push_function(const vm_exec& value) { lua_pushcfunction(state, value); }
            void push_color(const godot::Color& value) {
                create_table();
                push_number(value.r); set_table_field(1, -2);
                push_number(value.g); set_table_field(2, -2);
                push_number(value.b); set_table_field(3, -2);
                push_number(value.a); set_table_field(4, -2);
            }
            void push_vector2(const godot::Vector2& value) {
                create_table();
                push_number(value.x); set_table_field(1, -2);
                push_number(value.y); set_table_field(2, -2);
            }
            void push_vector2_array(const godot::PackedVector2Array& value) {
                create_table();
                for (int i = 0; i < value.size(); ++i) {
                    push_vector2(value[i]);
                    set_table_field(i + 1, -2);
                }
            }
            void push_vector3(const godot::Vector3& value) {
                create_table();
                push_number(value.x); set_table_field(1, -2);
                push_number(value.y); set_table_field(2, -2);
                push_number(value.z); set_table_field(3, -2);
            }
            void push_vector3_array(const godot::PackedVector3Array& value) {
                create_table();
                for (int i = 0; i < value.size(); ++i) {
                    push_vector3(value[i]);
                    set_table_field(i + 1, -2);
                }
            }


            // Containers //
            void create_table() { lua_newtable(state); }
            void create_metatable(const std::string& value) { luaL_newmetatable(state, value.c_str()); }
            void create_namespace(const std::string& nspace) {
                get_global(nspace);
                if (!is_table(-1)) {
                    pop();
                    create_table();
                    push_global(nspace);
                    get_global(nspace);
                }
            }
            Machine* create_thread() { 
                return new Machine(lua_newthread(state)); 
            }
            void create_object(const std::string& index, void* value) {
                create_userdata(value);
                set_metatable(index);
            }
            void create_userdata(void* value) {
                void** userdata = static_cast<void**>(lua_newuserdata(state, sizeof(void*)));
                *userdata = value;
            }


            // Error Handling & Execution //
            void log(const std::string& type, const std::string& message = "") {
                lua_Debug debug;
                lua_getstack(state, 1, &debug);
                lua_getinfo(state, "nSl", &debug);
                API::log(type, fmt::format("{}\n> Line: {}", message.empty() ? "N/A" : message, std::to_string(debug.currentline)));
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

            void hook(const std::string& mode) {
                for (auto& i : internal_apis) {
                    mode == "bind" ? i.bind(this) : i.inject(this);
                }
                for (auto& i : external_apis) {
                    mode == "bind" ? i.bind(this) : i.inject(this);
                }
            }

            void bind(const std::vector<std::string>& scope, const std::string& name, vm_bind exec) {
                auto heap_exec = new vm_bind(std::move(exec));
                create_namespace(scope[0]);
                for (std::size_t i = 1; i < scope.size(); ++i) {
                    get_table_field(scope[i], -1);
                    if (!is_table(-1)) {
                        pop();
                        create_table();
                        push(-1);
                        set_table_field(scope[i], -3);
                    }
                }
                lua_pushlightuserdata(state, heap_exec);
                lua_pushcclosure(state, [](vm_state* state) -> int {
                    auto exec = static_cast<vm_bind*>(lua_touserdata(state, lua_upvalueindex(1)));
                    auto vm = Machine::fetch_machine(state);
                    return vm->execute([&]() -> int {
                        return (*exec)(vm);
                    });
                }, 1);
                set_table_field(name, -2);
                pop(static_cast<int>(scope.size()));
            }


            // Utils //
            void push(int index = 1) { lua_pushvalue(state, index); }
            void pop(int count = 1) { lua_pop(state, count); }
            void move(Machine* target, int count = 1) { lua_xmove(state, target -> state, count); }
            bool pcall(int arguments, int returns) { return lua_pcall(state, arguments, returns, 0) == LUA_OK; }
            void set_table(int index = 1) { lua_settable(state, index); }
            void set_table_field(int field, int index = 1) { lua_seti(state, index, field); }
            void set_table_field(const std::string& field, int index = 1) { lua_setfield(state, index, field.c_str()); }
            void set_metatable(int index = 1) { lua_setmetatable(state, index);}
            void set_metatable(const std::string& index) { luaL_setmetatable(state, index.c_str()); }
    
            void set_reference(const std::string& name, int index = 1) {
                del_reference(name);
                push(index);
                reference.emplace(name, luaL_ref(state, LUA_REGISTRYINDEX));
            }

            void del_reference(const std::string& name) {
                if (!is_reference(name)) return;
                luaL_unref(state, LUA_REGISTRYINDEX, get_reference(name));
                reference.erase(name);
            }

            void resume(int count = 0) {
                if (!is_virtual()) return;
                int ncount;
                lua_resume(state, nullptr, count, &ncount);
                if (lua_status(state) != LUA_YIELD) delete this;
            }

            void pause(int count = 0) {
                if (!is_virtual()) return;
                lua_yield(state, count);
            }

            std::string to_string(int index = 1) {
                size_t length;
                const char* value = luaL_tolstring(state, index, &length);
                pop();
                return std::string(value, length);
            }
    
            bool compile_string(const std::string& raw) {
                if (raw.empty()) return false;
                if (luaL_loadstring(state, raw.c_str()) != LUA_OK) {
                    pop();
                    return false;
                }
                pop();
                return true;
            }

            int load_string(const std::string& raw, bool auto_load = true, bool use_env = false, int env_index = 1) {
                if (raw.empty()) return 0;
                if (luaL_loadstring(state, raw.c_str()) != LUA_OK) {
                    API::log(std::string(Vital::Log::Error::label), get_string(-1));
                    pop();
                    return 0;
                }
                if (use_env) {
                    push(env_index);
                    if (!lua_setupvalue(state, -2, 1)) pop();
                }
                if (auto_load && !pcall(0, LUA_MULTRET)) {
                    API::log(std::string(Vital::Log::Error::label), get_string(-1));
                    pop();
                    return 0;
                }
                return 1;
            }
    };
}