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
#include <Vital.sandbox/Manager/public/kit.h>


//////////////////////////////
// Vital: Sandbox: Machine //
//////////////////////////////

namespace Vital::Sandbox {
    class Machine : public Mixin<Machine> {
        protected:
            static vm_apis internal_apis;
            inline static std::mutex mutex;
            inline static vm_machines machines;
            inline static vm_env_cleaners env_cleaners;
            inline static std::vector<std::function<void()>> work_queue;

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
            vm_apis external_apis = {};
            std::function<void(Machine*, int nresults)> on_finish;


            // Helpers //
            inline static std::string make_reference(const std::string& scope, const std::string& name) { 
                return fmt::format("{}:{}", scope, name);
            }
        public:
            // Instantiators //
            Machine(vm_apis apis = {}) : state(luaL_newstate()), external_apis(std::move(apis)) {
                Tool::assert_main_thread("Machine::Machine");
                machines.emplace(state, this);
                for (auto& value : whitelist) {
                    luaL_requiref(state, value.name, value.func, 1);
                    pop(1);
                }
                for (auto& value : blacklist) {
                    push_nil();
                    push_global(value);
                }
                hook("bind");
                for (auto& [name, source] : Manager::Kit::fetch_modules("lua")) {
                    load_string(source, name);
                }
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
            bool is_nil(int index = 1) { return lua_isnoneornil(state, index); }
            bool is_bool(int index = 1) { return lua_isboolean(state, index); }
            bool is_number(int index = 1) { return lua_isnumber(state, index); }
            bool is_string(int index = 1) { return lua_isstring(state, index); }
            bool is_table(int index = 1) { return lua_istable(state, index); }
            bool is_thread(int index = 1) { return lua_isthread(state, index); }
            bool is_userdata(int index = 1) { return lua_isuserdata(state, index); }
            bool is_function(int index = 1) { return lua_isfunction(state, index); }
            bool is_reference(const std::string& scope, const std::string& name) const { return reference.find(make_reference(scope, name)) != reference.end(); }

            bool is_horizontal_alignment(int index = 1) {
                if (is_string(index)) return horizontal_alignment.count(get_string(index)) > 0;
                else if (is_number(index)) {
                    int value = get_int(index);
                    return value >= godot::HORIZONTAL_ALIGNMENT_LEFT && value <= godot::HORIZONTAL_ALIGNMENT_FILL;
                }
                return false;
            }

            bool is_vertical_alignment(int index = 1) {
                if (is_string(index)) return vertical_alignment.count(get_string(index)) > 0;
                else if (is_number(index)) {
                    int value = get_int(index);
                    return value >= godot::VERTICAL_ALIGNMENT_TOP && value <= godot::VERTICAL_ALIGNMENT_FILL;
                }
                return false;
            }

            bool is_color(int index = 1) {
                if (is_string(index)) return godot::Color::html_is_valid(Tool::to_godot_string(get_string(index)));
                return is_table(index) && get_length(index) >= 4;
            }

            bool is_vector2(int index = 1) { return is_table(index) && get_length(index) >= 2; }
            bool is_vector2_array(int index = 1) {
                if (!is_table(index)) return false;
                if (get_length(index) == 0) return false;
                get_table_field(1, index);
                bool result = is_vector2(-1);
                pop(1);
                return result;
            }

            bool is_vector3(int index = 1) { return is_table(index) && get_length(index) >= 3; }
            bool is_vector3_array(int index = 1) {
                if (!is_table(index)) return false;
                if (get_length(index) == 0) return false;
                get_table_field(1, index);
                bool result = is_vector3(-1);
                pop(1);
                return result;
            }


            // Getters //
            vm_state* get_state() const { return state; }
            int get_count() { return lua_gettop(state); }
            bool get_global(const std::string& index) { return lua_getglobal(state, index.c_str()); }
            bool get_bool(int index = 1) { return lua_toboolean(state, index); }
            int get_int(int index = 1) { return (int)lua_tonumber(state, index); }
            float get_float(int index = 1) { return (float)lua_tonumber(state, index); }
            double get_double(int index = 1) { return lua_tonumber(state, index); }
            std::string get_string(int index = 1) { return lua_tostring(state, index); }
            bool get_table(int index = 1) { return lua_gettable(state, index); }
            bool get_table_field(int value, int index = 1) { return lua_geti(state, index, value); }
            bool get_table_field(const std::string& value, int index = 1) { return lua_getfield(state, index, value.c_str()); }
            bool get_metatable(int index = 1) { return lua_getmetatable(state, index); }
            bool get_metatable(const std::string& index) { return luaL_getmetatable(state, index.c_str()); }
            vm_state* get_thread(int index = 1) { return lua_tothread(state, index); }
            void* get_userdata(int index = 1) { return lua_touserdata(state, index); }

            Machine* get_root() {
                Machine* root = this;
                while (root -> virtualized && root -> parent) root = root -> parent;
                return root;
            }

            int get_length(int index = 1) {
                lua_len(state, index);
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

            godot::HorizontalAlignment get_horizontal_alignment(int index = 1) {
                if (is_horizontal_alignment(index)) {
                    if (is_string(index)) return horizontal_alignment.find(get_string(index)) -> second;
                    return static_cast<godot::HorizontalAlignment>(get_int(index));
                }
                return godot::HORIZONTAL_ALIGNMENT_LEFT;
            }

            godot::VerticalAlignment get_vertical_alignment(int index = 1) {
                if (is_vertical_alignment(index)) {
                    if (is_string(index)) return vertical_alignment.find(get_string(index)) -> second;
                    return static_cast<godot::VerticalAlignment>(get_int(index));
                }
                return godot::VERTICAL_ALIGNMENT_TOP;
            }

            godot::Color get_color(int index = 1) {
                if (is_string(index)) {
                    auto html = Tool::to_godot_string(get_string(index));
                    if (godot::Color::html_is_valid(html)) return godot::Color::html(html);
                }
                godot::Color value = {1, 1, 1, 1};
                get_table_field(1, index); value.r = get_float(-1);
                get_table_field(2, index); value.g = get_float(-1);
                get_table_field(3, index); value.b = get_float(-1);
                get_table_field(4, index); value.a = get_float(-1);
                pop(4);
                return value;
            }

            godot::Vector2 get_vector2(int index = 1) {
                godot::Vector2 value = {0.0f, 0.0f};
                get_table_field(1, index); value.x = get_float(-1);
                get_table_field(2, index); value.y = get_float(-1);
                pop(2);
                return value;
            }

            godot::PackedVector2Array get_vector2_array(int index = 1) {
                godot::PackedVector2Array value;
                for (int i = 1; i <= get_length(index); ++i) {
                    get_table_field(i, index);
                    value.push_back(get_vector2(-1));
                    pop(1);
                }
                return value;
            }

            godot::Vector3 get_vector3(int index = 1) {
                godot::Vector3 value = {0.0f, 0.0f, 0.0f};
                get_table_field(1, index); value.x = get_float(-1);
                get_table_field(2, index); value.y = get_float(-1);
                get_table_field(3, index); value.z = get_float(-1);
                pop(3);
                return value;
            }

            godot::PackedVector3Array get_vector3_array(int index = 1) {
                godot::PackedVector3Array value;
                for (int i = 1; i <= get_length(index); ++i) {
                    get_table_field(i, index);
                    value.push_back(get_vector3(-1));
                    pop(1);
                }
                return value;
            }


            // Pushers //
            void push_global(const std::string& index) { lua_setglobal(state, index.c_str()); }
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

            void create_object(const std::string& index, void* value) {
                create_userdata(value);
                set_metatable(index);
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
                for (int index = level; lua_getstack(state, index, &debug); ++index) {
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

            void log(const std::string& type, const std::string& message = "") {
                const std::string source = fetch_source();
                const std::string err = message.empty() ? source : fmt::format("{}: {}", source, message);
                API::log(type, err);
                push_string(err);
                lua_error(state);
            }
            
            void log(const vm_error& e) {
                const std::string source = fetch_source();
                API::log(std::string(Tool::Log::error::label), fmt::format("{}: {}", source, e.detail));
                push_string(fmt::format("{}: {}", source, e.partial));
                lua_error(state);
            }

            template<typename F>
            int execute(F&& exec) {
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
                for (auto& i : internal_apis) { mode == "bind" ? i.bind(this) : i.inject(this); }
                for (auto& i : external_apis) { mode == "bind" ? i.bind(this) : i.inject(this); }
            }

            void bind(const std::vector<std::string>& scope, const std::string& name, vm_bind exec);


            // Utils //
            void push(int index = 1) { lua_pushvalue(state, index); }
            void pop(int count = 1) { lua_pop(state, count); }
            void move(Machine* target, int count = 1) { lua_xmove(state, target -> state, count); }
            void rotate(int index, int n) { lua_rotate(state, index, n); }
            void set_table(int index = 1) { lua_settable(state, index); }
            void set_table_field(int field, int index = 1) { lua_seti(state, index, field); }
            void set_table_field(const std::string& field, int index = 1) { lua_setfield(state, index, field.c_str()); }
            void set_metatable(int index = 1) { lua_setmetatable(state, index); }
            void set_metatable(const std::string& index) { luaL_setmetatable(state, index.c_str()); }

            void set_reference(const std::string& scope, const std::string& name, int index = 1) {
                Tool::assert_main_thread("Machine::set_reference");
                del_reference(scope, name);
                reference.emplace(make_reference(scope, name), set_raw_reference(index));
            }

            int set_raw_reference(int index = -1) {
                push(index);
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

            // Args Refs //
            int store_args_ref(int from_index) {
                create_table();
                int count = get_count() - from_index - 1;
                for (int i = 0; i <= count; ++i) {
                    push(from_index + i);
                    set_table_field(i + 1, -2);
                }
                return set_raw_reference(-1);
            }

            int push_args_ref(int args_ref) {
                get_raw_reference(args_ref);
                int table_idx = get_count();
                int n = get_length(table_idx);
                for (int i = 1; i <= n; ++i) get_table_field(i, table_idx);
                rotate(table_idx, -1);
                pop(1);
                return n;
            }

            // Serialization //
            Tool::StackValue collect_value(int index, std::unordered_set<const void*>& visited, int depth = 0) {
                switch (lua_type(state, index)) {
                    case LUA_TNIL: return Tool::StackValue(nullptr);
                    case LUA_TBOOLEAN: return Tool::StackValue(get_bool(index));
                    case LUA_TNUMBER: return Tool::StackValue(get_double(index));
                    case LUA_TSTRING: return Tool::StackValue(get_string(index));
                    case LUA_TTABLE: return Tool::StackValue(collect_table(index, visited, depth));
                    default: return Tool::StackValue(nullptr);
                }
            }

            std::shared_ptr<Tool::Stack> collect_table(int index, std::unordered_set<const void*>& visited, int depth = 0) {
                auto stack = std::make_shared<Tool::Stack>();
                if (depth > 32) return stack;

                const void* ptr = lua_topointer(state, index);
                if (!ptr || visited.count(ptr)) return stack;
                visited.insert(ptr);

                push_nil();
                while (lua_next(state, index) != 0) {
                    int key_type = lua_type(state, -2);
                    int val_type = lua_type(state, -1);
                    if (key_type == LUA_TNUMBER) {
                        int idx = get_int(-2);
                        if (idx >= 1) {
                            Tool::StackValue val = collect_value(get_count(), visited, depth + 1);
                            if (static_cast<int>(stack -> array.size()) < idx) stack -> array.resize(idx, Tool::StackValue(nullptr));
                            stack -> array[idx - 1] = val;
                        }
                    }
                    else if (key_type == LUA_TSTRING) {
                        if (val_type == LUA_TNIL ||
                            val_type == LUA_TBOOLEAN ||
                            val_type == LUA_TNUMBER ||
                            val_type == LUA_TSTRING ||
                            val_type == LUA_TTABLE
                        ) stack -> object[get_string(-2)] = collect_value(get_count(), visited, depth + 1);
                    }
                    pop(1);
                }
                visited.erase(ptr);
                return stack;
            }

            Tool::Stack collect_args(int from_index) {
                Tool::Stack payload;
                std::unordered_set<const void*> visited;
                int top = get_count();
                for (int i = from_index; i <= top; ++i) payload.array.emplace_back(collect_value(i, visited));
                return payload;
            }

            bool resume(int count = 0) {
                Tool::assert_main_thread("Machine::resume");
                if (!is_virtual()) return false;
                int nresults = 0;
                int result = lua_resume(state, nullptr, count, &nresults);
                if (result != LUA_OK && result != LUA_YIELD) {
                    if (get_count() > 0) {
                        API::log(std::string(Tool::Log::error::label), get_string(-1));
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
    
            std::string to_string(int index = 1) {
                size_t length;
                const char* value = luaL_tolstring(state, index, &length);
                pop(1);
                return std::string(value, length);
            }

            bool pcall(int arguments, int returns = LUA_MULTRET) {
                Tool::assert_main_thread("Machine::pcall");
                bool result = lua_pcall(state, arguments, returns, 0) == LUA_OK;
                if (!result) {
                    API::log(std::string(Tool::Log::error::label), get_string(-1));
                    pop(1);
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
        
            int load_string(const std::string& raw, const std::string& chunk_name = "", bool auto_load = true, bool use_env = false, int env_index = 1) {
                Tool::assert_main_thread("Machine::load_string");
                if (raw.empty()) return 0;
                const std::string name = chunk_name.empty() ? raw : ("@" + chunk_name);
                if (luaL_loadbuffer(state, raw.c_str(), raw.size(), name.c_str()) != LUA_OK) {
                    API::log(std::string(Tool::Log::error::label), get_string(-1));
                    pop(1);
                    return 0;
                }
                if (use_env) {
                    push(env_index);
                    if (!lua_setupvalue(state, -2, 1)) pop(1);
                }
                if (auto_load && !pcall(0)) return 0;
                return 1;
            }
    };
}