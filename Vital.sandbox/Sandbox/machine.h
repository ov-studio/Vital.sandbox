/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: machine.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Machine
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
    class Machine : public Mixin<Machine> {
        protected:
            static vm_apis apis;
            inline static std::mutex mutex;
            inline static vm_machines machines;
            inline static vm_env_cleaners env_cleaners;
            inline static std::vector<std::function<void()>> work_queue;
            inline static thread_local bool error_handled = false;

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

            inline static const std::unordered_map<std::string, godot::HorizontalAlignment> horizontal_alignment = {
                {"left", godot::HORIZONTAL_ALIGNMENT_LEFT},
                {"center", godot::HORIZONTAL_ALIGNMENT_CENTER},
                {"right", godot::HORIZONTAL_ALIGNMENT_RIGHT},
                {"fill", godot::HORIZONTAL_ALIGNMENT_FILL}
            };

            inline static const std::unordered_map<std::string, godot::VerticalAlignment> vertical_alignment = {
                {"top", godot::VERTICAL_ALIGNMENT_TOP},
                {"center", godot::VERTICAL_ALIGNMENT_CENTER},
                {"bottom", godot::VERTICAL_ALIGNMENT_BOTTOM},
                {"fill", godot::VERTICAL_ALIGNMENT_FILL}
            };
        private:
            bool virtualized = false;
            vm_state* state = nullptr;
            Machine* parent = nullptr;
            std::unordered_set<Machine*> children = {};
            vm_refs reference = {};
            std::function<void(Machine*, int nresults)> on_finish;


            // Helpers //
            static void load_modules(Machine* vm);
            inline static std::string make_reference(const std::string& scope, const std::string& name) { 
                return fmt::format("{}:{}", scope, name);
            }
        public:
            // Instantiators //
            Machine() {
                Tool::assert_main_thread("Machine::Machine");
                state = luaL_newstate();
                machines.emplace(state, this);
                for (auto& value : whitelist) {
                    luaL_requiref(state, value.name, value.func, 1);
                    pop(1);
                }
                for (auto& value : blacklist) {
                    push_nil();
                    push_global(value);
                }
                hook("init");
                hook("bind");
                load_modules(this);
                hook("inject");
            }

            Machine(vm_state* thread, Machine* parent) : state(thread), virtualized(true), parent(parent) {
                Tool::assert_main_thread("Machine::Machine(thread)");
                machines.emplace(state, this);
            }

            ~Machine() {
                Tool::assert_main_thread("Machine::~Machine");
                if (!state) return;
                if (!virtualized) {
                    for (auto* child : children) {
                        child -> parent = nullptr;
                        machines.erase(child -> state);
                        child -> state = nullptr;
                        delete child;
                    }
                    children.clear();
                    lua_close(state);
                }
                else if (parent) {
                    parent -> children.erase(this);
                    parent = nullptr;
                }
                machines.erase(state);
                state = nullptr;
            }

            
            // Managers //
            static Machine* to_machine(void* vm) { return static_cast<Machine*>(vm); }
            static const vm_machines fetch_machines() { return machines; }
            
            static Machine* fetch_machine(vm_state* state) {
                auto it = machines.find(state);
                return it != machines.end() ? it -> second : nullptr;
            }

            static void enqueue(std::function<void()> exec) {
                std::lock_guard<std::mutex> lock(mutex);
                work_queue.push_back(std::move(exec));
            }

            static void next_tick(std::function<void()> exec);

            static void drain() {
                std::vector<std::function<void()>> work;
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    std::swap(work, work_queue);
                }
                for (auto& exec : work) exec();
            }


            // Checkers //
            bool is_virtual() const { return virtualized; }
            bool is_nil(int idx = 1) { return get_type(idx) <= 0; }
            bool is_bool(int idx = 1) { return get_type(idx) == LUA_TBOOLEAN; }
            bool is_number(int idx = 1) { return get_type(idx) == LUA_TNUMBER; }
            bool is_string(int idx = 1) { return get_type(idx) == LUA_TSTRING; }
            bool is_table(int idx = 1) { return get_type(idx) == LUA_TTABLE; }
            bool is_thread(int idx = 1) { return get_type(idx) == LUA_TTHREAD; }
            bool is_userdata(int idx = 1) { return (get_type(idx) == LUA_TUSERDATA) || (get_type(idx) == LUA_TLIGHTUSERDATA); }
            bool is_pointer(int idx = 1) { return lua_topointer(state, idx) != nullptr; }
            bool is_function(int idx = 1) { return get_type(idx) == LUA_TFUNCTION; }
            bool is_reference(const std::string& scope, const std::string& name) const { return reference.find(make_reference(scope, name)) != reference.end(); }

            bool is_horizontal_alignment(int idx = 1) {
                if (is_string(idx)) return horizontal_alignment.count(get_string(idx)) > 0;
                else if (is_number(idx)) {
                    int value = get_int(idx);
                    return value >= godot::HORIZONTAL_ALIGNMENT_LEFT && value <= godot::HORIZONTAL_ALIGNMENT_FILL;
                }
                return false;
            }

            bool is_vertical_alignment(int idx = 1) {
                if (is_string(idx)) return vertical_alignment.count(get_string(idx)) > 0;
                else if (is_number(idx)) {
                    int value = get_int(idx);
                    return value >= godot::VERTICAL_ALIGNMENT_TOP && value <= godot::VERTICAL_ALIGNMENT_FILL;
                }
                return false;
            }

            bool is_color(int idx = 1) {
                if (is_string(idx)) return godot::Color::html_is_valid(Tool::to_godot_string(get_string(idx)));
                return is_table(idx) && get_length(idx) >= 4;
            }

            bool is_vector2(int idx = 1) { return is_table(idx) && get_length(idx) >= 2; }
            bool is_vector2_array(int idx = 1) {
                if (!is_table(idx)) return false;
                if (get_length(idx) == 0) return false;
                get_table_field(1, idx);
                bool result = is_vector2(-1);
                pop(1);
                return result;
            }

            bool is_vector3(int idx = 1) { return is_table(idx) && get_length(idx) >= 3; }
            bool is_vector3_array(int idx = 1) {
                if (!is_table(idx)) return false;
                if (get_length(idx) == 0) return false;
                get_table_field(1, idx);
                bool result = is_vector3(-1);
                pop(1);
                return result;
            }


            // Getters //
            vm_state* get_state() const { return state; }
            int get_count() { return lua_gettop(state); }
            int get_type(int idx = 1) { return lua_type(state, idx); }
            bool get_global(const std::string& idx) { return lua_getglobal(state, idx.c_str()); }
            bool get_bool(int idx = 1) { return lua_toboolean(state, idx); }
            int get_int(int idx = 1) { return (int)lua_tonumber(state, idx); }
            float get_float(int idx = 1) { return (float)lua_tonumber(state, idx); }
            double get_double(int idx = 1) { return lua_tonumber(state, idx); }
            std::string get_string(int idx = 1) { return lua_tostring(state, idx); }
            bool get_table(int idx = 1) { return lua_gettable(state, idx); }
            bool get_table_field(int value, int idx = 1) { return lua_geti(state, idx, value); }
            bool get_table_field(const std::string& value, int idx = 1) { return lua_getfield(state, idx, value.c_str()); }
            bool get_metatable(int idx = 1) { return lua_getmetatable(state, idx); }
            bool get_metatable(const std::string& idx) { return luaL_getmetatable(state, idx.c_str()); }
            vm_state* get_thread(int idx = 1) { return lua_tothread(state, idx); }
            void* get_userdata(int idx = 1) { return lua_touserdata(state, idx); }
            const void* get_pointer(int idx = 1) { return lua_topointer(state, idx); }

            Machine* get_root() {
                Machine* root = this;
                while (root -> virtualized && root -> parent) root = root -> parent;
                return root;
            }

            int get_length(int idx = 1) {
                lua_len(state, idx);
                int result = get_int(-1);
                pop(1);
                return result;
            }

            int get_reference(const std::string& scope, const std::string& name, bool push_to_stack = false) {
                if (!push_to_stack) return reference.at(make_reference(scope, name));
                get_raw_reference(reference.at(make_reference(scope, name)));
                return 0;
            }

            int get_raw_reference(int ref) {
                lua_rawgeti(state, LUA_REGISTRYINDEX, ref);
                return 0;
            }

            godot::HorizontalAlignment get_horizontal_alignment(int idx = 1) {
                if (is_horizontal_alignment(idx)) {
                    if (is_string(idx)) return horizontal_alignment.find(get_string(idx)) -> second;
                    return static_cast<godot::HorizontalAlignment>(get_int(idx));
                }
                return godot::HORIZONTAL_ALIGNMENT_LEFT;
            }

            godot::VerticalAlignment get_vertical_alignment(int idx = 1) {
                if (is_vertical_alignment(idx)) {
                    if (is_string(idx)) return vertical_alignment.find(get_string(idx)) -> second;
                    return static_cast<godot::VerticalAlignment>(get_int(idx));
                }
                return godot::VERTICAL_ALIGNMENT_TOP;
            }

            godot::Color get_color(int idx = 1) {
                if (is_string(idx)) {
                    auto html = Tool::to_godot_string(get_string(idx));
                    if (godot::Color::html_is_valid(html)) return godot::Color::html(html);
                }
                godot::Color value = {1, 1, 1, 1};
                get_table_field(1, idx); value.r = get_float(-1);
                get_table_field(2, idx); value.g = get_float(-1);
                get_table_field(3, idx); value.b = get_float(-1);
                get_table_field(4, idx); value.a = get_float(-1);
                pop(4);
                return value;
            }

            godot::Vector2 get_vector2(int idx = 1) {
                godot::Vector2 value = {0.0f, 0.0f};
                get_table_field(1, idx); value.x = get_float(-1);
                get_table_field(2, idx); value.y = get_float(-1);
                pop(2);
                return value;
            }

            godot::PackedVector2Array get_vector2_array(int idx = 1) {
                godot::PackedVector2Array value;
                for (int i = 1; i <= get_length(idx); ++i) {
                    get_table_field(i, idx);
                    value.push_back(get_vector2(-1));
                    pop(1);
                }
                return value;
            }

            godot::Vector3 get_vector3(int idx = 1) {
                godot::Vector3 value = {0.0f, 0.0f, 0.0f};
                get_table_field(1, idx); value.x = get_float(-1);
                get_table_field(2, idx); value.y = get_float(-1);
                get_table_field(3, idx); value.z = get_float(-1);
                pop(3);
                return value;
            }

            godot::PackedVector3Array get_vector3_array(int idx = 1) {
                godot::PackedVector3Array value;
                for (int i = 1; i <= get_length(idx); ++i) {
                    get_table_field(i, idx);
                    value.push_back(get_vector3(-1));
                    pop(1);
                }
                return value;
            }


            // Pushers //
            void push_global(const std::string& idx) { lua_setglobal(state, idx.c_str()); }
            void push_nil() { lua_pushnil(state); }
            void push_bool(bool value) { lua_pushboolean(state, value); }
            void push_number(int value) { lua_pushinteger(state, value); }
            void push_number(int64_t value) { lua_pushinteger(state, value); }
            void push_number(float value) { lua_pushnumber(state, value); }
            void push_number(double value) { lua_pushnumber(state, value); }
            void push_string(const std::string& value) { lua_pushstring(state, value.c_str()); }
            void push_userdata(void* value) { lua_pushlightuserdata(state, value); }
            void push_function(const vm_exec& value) { lua_pushcfunction(state, value); }

            void push_horizontal_alignment(godot::HorizontalAlignment value) {
                for (auto& it : horizontal_alignment) {
                    if (it.second == value) {
                        push_value(it.first);
                        return;
                    }
                }
                push_value("left");
            }

            void push_vertical_alignment(godot::VerticalAlignment value) {
                for (auto& it : vertical_alignment) {
                    if (it.second == value) {
                        push_value(it.first);
                        return;
                    }
                }
                push_value("top");
            }

            void push_color(const godot::Color& value) {
                create_table();
                push_value(value.r); set_table_field(1, -2);
                push_value(value.g); set_table_field(2, -2);
                push_value(value.b); set_table_field(3, -2);
                push_value(value.a); set_table_field(4, -2);
            }

            void push_vector2(const godot::Vector2& value) {
                create_table();
                push_value(value.x); set_table_field(1, -2);
                push_value(value.y); set_table_field(2, -2);
            }

            void push_vector2_array(const godot::PackedVector2Array& value) {
                create_table();
                for (int i = 0; i < value.size(); ++i) {
                    push_value(value[i]);
                    set_table_field(i + 1, -2);
                }
            }

            void push_vector3(const godot::Vector3& value) {
                create_table();
                push_value(value.x); set_table_field(1, -2);
                push_value(value.y); set_table_field(2, -2);
                push_value(value.z); set_table_field(3, -2);
            }

            void push_vector3_array(const godot::PackedVector3Array& value) {
                create_table();
                for (int i = 0; i < value.size(); ++i) {
                    push_value(value[i]);
                    set_table_field(i + 1, -2);
                }
            }


            // Containers //
            void create_table() { lua_newtable(state); }
            void create_metatable(const std::string& value) { luaL_newmetatable(state, value.c_str()); }

            void create_namespace(const std::string& nspace) {
                get_global(nspace);
                if (!is_table(-1)) {
                    pop(1);
                    create_table();
                    push_global(nspace);
                    get_global(nspace);
                }
            }

            void create_object(const std::string& idx, void* value) {
                create_userdata(value);
                set_metatable(idx);
            }

            void create_userdata(void* value) {
                void** userdata = static_cast<void**>(lua_newuserdata(state, sizeof(void*)));
                *userdata = value;
            }

            Machine* create_thread() {
                Tool::assert_main_thread("Machine::create_thread");
                auto* thread = new Machine(lua_newthread(state), this);
                children.emplace(thread);
                return thread;
            }


            // Environment //
            void create_environment(const std::string& id) {
                Tool::assert_main_thread("Machine::create_environment");
                create_table();
                create_table();
                lua_pushglobaltable(state);
                set_table_field("__index", -2);
                set_metatable(-2);
                push(-1);
                push_string(id);
                lua_rawset(state, LUA_REGISTRYINDEX);
                set_reference("env", id, -1);
            }

            static void register_environment_cleaner(vm_env_cleaner exec) {
                env_cleaners.push_back(std::move(exec));
            }
    
            std::string get_environment_id(int level = 0) {
                Tool::assert_main_thread("Machine::get_environment_id");
                lua_Debug debug;
                for (int idx = level; lua_getstack(state, idx, &debug); ++idx) {
                    lua_getinfo(state, "f", &debug);
                    const char* upname = lua_getupvalue(state, -1, 1);
                    lua_remove(state, -2);
                    if (!upname) continue;
                    if (!is_table(-1)) { pop(1); continue; }
                    lua_rawget(state, LUA_REGISTRYINDEX);
                    if (is_string(-1)) {
                        std::string result = get_string(-1);
                        pop(1);
                        return result;
                    }
                    pop(1);
                }
                return "";
            }

            void clear_environment_id(const std::string& id) {
                Tool::assert_main_thread("Machine::clear_environment_id");
                if (!is_reference("env", id)) return;
                get_reference("env", id, true);
                push_nil();
                lua_rawset(state, LUA_REGISTRYINDEX);
                del_reference("env", id);
                lua_gc(state, LUA_GCCOLLECT, 0);
                for (auto& clean : env_cleaners) clean(id);
            }


            // Context Handles //
            std::string fetch_source() {
                lua_Debug debug;
                std::string source = "?:0";
                if (lua_getstack(state, 1, &debug)) {
                    lua_getinfo(state, "Sl", &debug);
                    const char* src = debug.source;
                    source = fmt::format("{}:{}",
                        (src && src[0] == '@') ? src + 1 : (src ? src : "?"),
                        debug.currentline);
                }
                return source;
            }

            void log(const std::string& type, const std::string& message = "", bool halt = true) {
                const std::string source = fetch_source();
                const std::string err = message.empty() ? source : fmt::format("{}: {}", source, message);
                Tool::print(type, err);
                error_handled = true;
                push_string(err);
                if (halt) lua_error(state);
            }
            
            void log(const vm_error& e, bool halt = true) {
                const std::string source = fetch_source();
                Tool::print(std::string(Tool::Log::error::label), fmt::format("{}: {}", source, e.detail));
                error_handled = true;
                push_string(fmt::format("{}: {}", source, e.partial));
                if (halt) lua_error(state);
            }

            template<typename F>
            int execute(F&& exec) {
                error_handled = false;
                try { return exec(); }
                catch (const vm_error& e) { log(e); }
                catch (const Tool::Log::info& e) { log(std::string(Tool::Log::info::label), e.what()); }
                catch (const Tool::Log::warn& e) { log(std::string(Tool::Log::warn::label), e.what()); }
                catch (const Tool::Log::error& e) { log(std::string(Tool::Log::error::label), e.what()); }
                catch (const std::runtime_error& e) { log(std::string(Tool::Log::error::label), e.what()); }
                catch (...) { log(std::string(Tool::Log::error::label), "unknown exception"); }
                return 0;
            }

            void hook(const std::string& mode) {
                Tool::assert_main_thread("Machine::hook");
                for (auto& i : apis) {
                    if (mode == "init") i.init(this);
                    else if (mode == "bind") i.bind(this);
                    else i.inject(this);
                }
            }

            void bind(const std::vector<std::string>& scope, const std::string& name, vm_bind exec);


            // Misc //
            void push(int idx = 1) { lua_pushvalue(state, idx); }
            void pop(int count = 1) { lua_pop(state, count); }
            bool next(int idx = 1) { return lua_next(state, idx) != 0; }
            void move(Machine* target, int count = 1) { lua_xmove(state, target -> state, count); }
            void rotate(int idx, int n) { lua_rotate(state, idx, n); }
            void set_table(int idx = 1) { lua_settable(state, idx); }
            void set_table_field(int field, int idx = 1) { lua_seti(state, idx, field); }
            void set_table_field(const std::string& field, int idx = 1) { lua_setfield(state, idx, field.c_str()); }
            void set_metatable(int idx = 1) { lua_setmetatable(state, idx); }
            void set_metatable(const std::string& idx) { luaL_setmetatable(state, idx.c_str()); }

            void set_reference(const std::string& scope, const std::string& name, int idx = 1) {
                Tool::assert_main_thread("Machine::set_reference");
                del_reference(scope, name);
                reference.emplace(make_reference(scope, name), set_raw_reference(idx));
            }

            int set_raw_reference(int idx = 1) {
                push(idx);
                return luaL_ref(state, LUA_REGISTRYINDEX);
            }

            void del_reference(const std::string& scope, const std::string& name) {
                Tool::assert_main_thread("Machine::del_reference");
                if (!is_reference(scope, name)) return;
                del_raw_reference(get_reference(scope, name));
                reference.erase(make_reference(scope, name));
            }

            void del_raw_reference(int ref) {
                if (ref == LUA_NOREF) return;
                luaL_unref(state, LUA_REGISTRYINDEX, ref);
            }

            int store_args(int idx) {
                create_table();
                int count = get_count() - idx - 1;
                for (int i = 0; i <= count; ++i) {
                    push(idx + i);
                    set_table_field(i + 1, -2);
                }
                return set_raw_reference(-1);
            }

            int push_args(int ref) {
                get_raw_reference(ref);
                int table_idx = get_count();
                int n = get_length(table_idx);
                for (int i = 1; i <= n; ++i) get_table_field(i, table_idx);
                rotate(table_idx, -1);
                pop(1);
                return n;
            }
    
            bool resume(int count = 0) {
                Tool::assert_main_thread("Machine::resume");
                if (!is_virtual()) return false;
                int nresults = 0;
                int result = lua_resume(state, nullptr, count, &nresults);
                if (result != LUA_OK && result != LUA_YIELD) {
                    if (get_count() > 0) {
                        if (!error_handled) Tool::print(std::string(Tool::Log::error::label), get_string(-1));
                        error_handled = false;
                        pop(1);
                    }
                }
                if (result != LUA_YIELD) {
                    if (on_finish) {
                        on_finish(this, nresults);
                        on_finish = nullptr;
                    }
                    delete this;
                    return false;
                }
                return true;
            }

            void pause(int count = 0) {
                Tool::assert_main_thread("Machine::pause");
                if (!is_virtual()) return;
                lua_yield(state, count);
            }

            void set_finish_hook(std::function<void(Machine*, int)> exec) {
                if (!is_virtual()) return;
                on_finish = std::move(exec);
            }
    
            std::string to_string(int idx = 1) {
                size_t length;
                const char* value = luaL_tolstring(state, idx, &length);
                pop(1);
                return std::string(value, length);
            }

            bool pcall(int arguments, int returns = LUA_MULTRET) {
                Tool::assert_main_thread("Machine::pcall");
                return lua_pcall(state, arguments, returns, 0) == LUA_OK;
            }

            bool call(int arguments, int returns = LUA_MULTRET, bool protected_call = true) {
                Tool::assert_main_thread("Machine::call");
                bool result = pcall(arguments, returns);
                if (!result) {
                    if (protected_call) {
                        if (!error_handled) Tool::print(std::string(Tool::Log::error::label), get_string(-1));
                        error_handled = false;
                        pop(1);
                    }
                    else {
                        const std::string err = get_string(-1);
                        pop(1);
                        log(std::string(Tool::Log::error::label), err, false);
                        error_handled = true;
                        lua_error(state);
                    }
                }
                return result;
            }
            
            std::string compile_string(const std::string& raw, const std::string& chunk_name = "") {
                Tool::assert_main_thread("Machine::compile_string");
                if (raw.empty()) return "empty source";
                const std::string name = chunk_name.empty() ? raw : ("@" + chunk_name);
                if (luaL_loadbuffer(state, raw.c_str(), raw.size(), name.c_str()) != LUA_OK) {
                    std::string err = get_string(-1);
                    pop(1);
                    return err;
                }
                pop(1);
                return "";
            }
        
            int load_string(const std::string& raw, const std::string& chunk_name = "", bool auto_load = true, bool use_env = false, int env_idx = 1) {
                Tool::assert_main_thread("Machine::load_string");
                if (raw.empty()) return 0;
                const std::string name = chunk_name.empty() ? raw : ("@" + chunk_name);
                if (luaL_loadbuffer(state, raw.c_str(), raw.size(), name.c_str()) != LUA_OK) {
                    Tool::print(std::string(Tool::Log::error::label), get_string(-1));
                    pop(1);
                    return 0;
                }
                if (use_env) {
                    push(env_idx);
                    if (!lua_setupvalue(state, -2, 1)) pop(1);
                }
                if (auto_load && !call(0)) return 0;
                return 1;
            }
    };
}