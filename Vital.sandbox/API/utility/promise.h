/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: promise.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Promise APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/core.h>


//////////////////////////
// Vital: API: Promise //
//////////////////////////

namespace Vital::Sandbox::API {
    struct Promise : vm_module {
        inline static const std::string base_name = "promise";

        enum class State { Pending, Resolved, Rejected };

        // Plain C++ value — safe to copy into deferred lambdas with no lua_State* dependency.
        struct SerialValue {
            int type { LUA_TNIL };
            bool b {};
            bool is_int {};
            lua_Integer i {};
            lua_Number n {};
            std::string s {};

            void push(lua_State* L) const {
                switch (type) {
                    case LUA_TBOOLEAN: {
                        lua_pushboolean(L, b ? 1 : 0);
                        break;
                    }
                    case LUA_TNUMBER: {
                        if (is_int) lua_pushinteger(L, i);
                        else lua_pushnumber(L, n);
                        break;
                    }
                    case LUA_TSTRING: {
                        lua_pushlstring(L, s.c_str(), s.size()); break;
                    }
                    default: {
                        lua_pushnil(L);
                        break;
                    }
                }
            }

            static SerialValue from(lua_State* L, int index) {
                SerialValue v;
                v.type = ::lua_type(L, index);
                switch (v.type) {
                    case LUA_TBOOLEAN:
                        v.b = lua_toboolean(L, index) != 0;
                        break;
                    case LUA_TNUMBER:
                        v.is_int = lua_isinteger(L, index) != 0;
                        if (v.is_int) v.i = lua_tointeger(L, index);
                        else          v.n = lua_tonumber(L, index);
                        break;
                    case LUA_TSTRING: {
                        size_t len;
                        const char* s = lua_tolstring(L, index, &len);
                        v.s = std::string(s, len);
                        break;
                    }
                    default: break;
                }
                return v;
            }
        };

        struct WaitingThread {
            int thread_instance_id = -1;
        };

        struct Instance {
            int id {};
            std::string env;
            std::atomic<bool> destroyed { false };
            State state { State::Pending };
            Machine* vm = nullptr;
            void** userdata = nullptr;
            std::vector<WaitingThread> waiting;
            bool resolved = false;
            std::vector<SerialValue> serial_values;
            std::string reference() const { return fmt::format("{}:{}", base_name, id); }
        };
        inline static std::unordered_map<int, std::shared_ptr<Instance>> buffer;
        inline static std::atomic<int> next_id { 1 };
        inline static std::mutex mutex;

        using ResumeDispatcher = std::function<void(int, bool, const std::vector<SerialValue>&)>;
        inline static ResumeDispatcher resume_dispatcher;
        static void register_resume_dispatcher(ResumeDispatcher fn) {
            resume_dispatcher = std::move(fn);
        }

        static std::shared_ptr<Instance> fetch_instance(int id) {
            std::lock_guard<std::mutex> lock(mutex);
            auto it = buffer.find(id);
            return it != buffer.end() ? it -> second : nullptr;
        }

        static void clean_instance(std::shared_ptr<Instance> instance) {
            if (!instance) return;
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (buffer.find(instance -> id) == buffer.end()) return;
                buffer.erase(instance -> id);
            }
            if (instance -> vm) {
                vm_module::release_userdata_ptr(instance -> userdata);
                instance -> vm -> del_reference(instance -> reference());
                instance -> vm = nullptr;
            }
        }

        // TODO: Get stack instead of vm to save cost creating unnecessary machines...
        static void settle(std::shared_ptr<Instance> instance, State result_state, Machine* vm, int args_start, int args_count) {
            if (!instance || instance -> destroyed || instance -> state != State::Pending || !vm) return;

            instance -> state = result_state;
            instance -> resolved = (result_state == State::Resolved);
            instance -> serial_values.clear();
            instance -> serial_values.reserve(args_count);
            lua_State* src = vm -> get_state();
            for (int i = 0; i < args_count; ++i) instance -> serial_values.push_back(SerialValue::from(src, args_start + i));

            auto waiting = instance -> waiting;
            instance -> waiting.clear();
            bool resolved = instance -> resolved;
            std::vector<SerialValue> values = instance -> serial_values;
            std::vector<int> thread_ids;
            thread_ids.reserve(waiting.size());
            for (auto& wt : waiting) {
                if (wt.thread_instance_id >= 0) thread_ids.push_back(wt.thread_instance_id);
            }
            if (!Promise::resume_dispatcher) return;
            for (int tid : thread_ids) Promise::resume_dispatcher(tid, resolved, values);
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Promise>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                std::string env = vm -> get_environment_id();
                auto instance = std::make_shared<Instance>();
                instance -> id = next_id.fetch_add(1);
                instance -> env = env;
                instance -> vm = vm;
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    buffer[instance -> id] = instance;
                }
                vm -> create_object(base_name, instance.get());
                instance -> userdata = vm_module::get_userdata_ptr(vm, -1);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, base_name, "destroy", [](auto vm, auto self, auto& id) -> int {
                if (self -> destroyed) { vm -> push_value(false); return 1; }
                self -> destroyed = true;
                auto instance = fetch_instance(self -> id);
                if (instance) clean_instance(instance);
                vm_module::release_userdata(vm, 1);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "is_pending", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(!self -> destroyed && self -> state == State::Pending);
                return 1;
            });
        
            vm_module::bind_method<Instance>(vm, base_name, "resolve", [](auto vm, auto self, auto& id) -> int {
                if (self -> destroyed || self -> state != State::Pending) { vm -> push_value(false); return 1; }
                auto instance = fetch_instance(self -> id);
                if (!instance) vm -> push_value(false);
                else {
                    settle(instance, State::Resolved, vm, 2, vm -> get_count() - 1);
                    vm -> push_value(true);
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "reject", [](auto vm, auto self, auto& id) -> int {
                if (self -> destroyed || self -> state != State::Pending) { vm -> push_value(false); return 1; }
                auto instance = fetch_instance(self -> id);
                if (!instance) vm -> push_value(false);
                else {
                    settle(instance, State::Rejected, vm, 2, vm -> get_count() - 1);
                    vm -> push_value(true);
                }
                return 1;
            });
        }

        static void clean(const std::string& env) {
            std::vector<std::shared_ptr<Instance>> to_clean;
            {
                std::lock_guard<std::mutex> lock(mutex);
                for (auto& [id, instance] : buffer) {
                    if (instance -> env == env) to_clean.push_back(instance);
                }
            }
            for (auto& instance : to_clean) {
                instance -> destroyed = true;
                instance -> waiting.clear();
            }
            for (auto& instance : to_clean) clean_instance(instance);
        }
    };
}