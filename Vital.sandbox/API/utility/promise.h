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

        struct WaitingThread {
            Machine* thread_vm = nullptr; // the coroutine to resume
            Machine* owner_vm  = nullptr; // the owner vm (for pushing values)
        };

        struct Instance {
            int id;
            std::string env;
            std::atomic<bool> destroyed{false};
            State state{State::Pending};
            Machine* vm = nullptr;
            void** userdata = nullptr;
            std::vector<WaitingThread> waiting;
            int result_ref   = LUA_NOREF;
            int result_count = 0;          // number of values packed into result_ref table
            bool resolved = false;         // true = resolved, false = rejected
            std::string reference() const { return fmt::format("{}:{}", base_name, id); }
        };
        inline static std::unordered_map<int, std::shared_ptr<Instance>> buffer;
        inline static std::atomic<int> next_id{1};
        inline static std::mutex mutex;

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
            if (instance -> result_ref != LUA_NOREF && instance -> vm) {
                luaL_unref(instance -> vm -> get_state(), LUA_REGISTRYINDEX, instance -> result_ref);
                instance -> result_ref = LUA_NOREF;
            }
            vm_module::release_userdata_ptr(instance -> userdata);
            instance -> vm -> del_reference(instance -> reference());
        }

        static void settle(std::shared_ptr<Instance> instance, State result_state, Machine* vm, int args_start, int args_count) {
            if (!instance || instance -> state != State::Pending) return;
            instance -> state    = result_state;
            instance -> resolved = (result_state == State::Resolved);
            instance -> result_count = args_count;

            // Store a result table in the promise vm's registry for the already-settled
            // fast path in await (same-vm access, no xmove needed there).
            vm -> create_table();
            for (int i = 0; i < args_count; ++i) {
                vm -> push(args_start + i);
                vm -> set_table_field(i + 1, -2);
            }
            instance -> result_ref = luaL_ref(vm -> get_state(), LUA_REGISTRYINDEX);

            auto waiting = instance -> waiting;
            instance -> waiting.clear();

            // For each waiting thread, store a copy of the result values as a table
            // on owner_vm (which shares the same root Lua state as thread_vm).
            // We do this NOW, before the deferred fires, while vm's stack is intact.
            struct ResumeEntry {
                Machine* thread_vm;
                Machine* owner_vm;
                int      ref;   // result table ref stored in owner_vm's registry
            };
            std::vector<ResumeEntry> entries;
            entries.reserve(waiting.size());
            for (auto& wt : waiting) {
                if (!wt.thread_vm || !wt.owner_vm) continue;
                // Build result table on owner_vm — same root as thread_vm, safe to xmove
                wt.owner_vm -> create_table();
                for (int i = 0; i < args_count; ++i) {
                    vm -> push(args_start + i);                         // push value on settle vm
                    lua_xmove(vm -> get_state(), wt.owner_vm -> get_state(), 1); // move to owner_vm
                    wt.owner_vm -> set_table_field(i + 1, -2);
                }
                int ref = luaL_ref(wt.owner_vm -> get_state(), LUA_REGISTRYINDEX);
                entries.push_back({ wt.thread_vm, wt.owner_vm, ref });
            }

            Vital::Engine::Core::get_singleton() -> push_deferred([instance, entries]() {
                for (auto& e : entries) {
                    if (!e.thread_vm) continue;

                    int n = instance -> result_count;

                    // Push resolved bool directly onto thread_vm
                    e.thread_vm -> push_value(instance -> resolved);

                    // Unpack result table from owner_vm (same root as thread_vm) onto
                    // owner_vm stack, then xmove the n values to thread_vm — safe.
                    if (n > 0) {
                        lua_State* src = e.owner_vm -> get_state();
                        lua_rawgeti(src, LUA_REGISTRYINDEX, e.ref); // push table
                        for (int i = 1; i <= n; ++i) lua_rawgeti(src, -1, i); // push fields
                        lua_remove(src, -(n + 1));                              // remove table
                        lua_xmove(src, e.thread_vm -> get_state(), n);
                    }
                    // Release the per-thread ref now that we've consumed it
                    luaL_unref(e.owner_vm -> get_state(), LUA_REGISTRYINDEX, e.ref);

                    // Resume with (bool, v1, v2, ...) — 1 + n values
                    e.thread_vm -> resume(1 + n);
                }
            });
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
            // promise:resolve(...)
            vm_module::bind_method<Instance>(vm, base_name, "resolve", [](auto vm, auto self, auto& id) -> int {
                if (!self || self -> destroyed || self -> state != State::Pending) { vm -> push_value(false); return 1; }
                auto instance = fetch_instance(self -> id);
                if (!instance) { vm -> push_value(false); return 1; }
                int args = vm -> get_count() - 1;
                settle(instance, State::Resolved, vm, 2, args);
                vm -> push_value(true);
                return 1;
            });

            // promise:reject(...)
            vm_module::bind_method<Instance>(vm, base_name, "reject", [](auto vm, auto self, auto& id) -> int {
                if (!self || self -> destroyed || self -> state != State::Pending) { vm -> push_value(false); return 1; }
                auto instance = fetch_instance(self -> id);
                if (!instance) { vm -> push_value(false); return 1; }
                int args = vm -> get_count() - 1;
                settle(instance, State::Rejected, vm, 2, args);
                vm -> push_value(true);
                return 1;
            });

            // promise:is_pending()
            vm_module::bind_method<Instance>(vm, base_name, "is_pending", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self && !self -> destroyed && self -> state == State::Pending);
                return 1;
            });

            // promise:destroy()
            vm_module::bind_method<Instance>(vm, base_name, "destroy", [](auto vm, auto self, auto& id) -> int {
                if (!self || self -> destroyed) { vm -> push_value(false); return 1; }
                self -> destroyed = true;
                auto instance = fetch_instance(self -> id);
                if (instance) Vital::Engine::Core::get_singleton() -> push_deferred([instance]() { clean_instance(instance); });
                vm_module::release_userdata(vm, 1);
                vm -> push_value(true);
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
                clean_instance(instance);
            }
        }
    };
}