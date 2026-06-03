/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: event.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Event APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/API/utility/promise.h>
#include <Vital.sandbox/API/utility/thread.h>


////////////////////////
// Vital: API: Event //
////////////////////////

namespace Vital::Sandbox::API {
    struct Event : vm_module {
        inline static const std::string base_name = "event";

        // Per-event handler registry
        struct Handler {
            int exec_ref = LUA_NOREF;
            bool async = false;
            bool is_callback = false;
            int subscription_limit = 0;
            int subscription_count = 0;
        };

        struct EventEntry {
            // callback mode: only one handler
            std::optional<Handler> callback_handler;
            // emit mode: ordered priority list + normal map
            std::vector<std::pair<int /*ref*/, Handler>> priority_handlers;
            std::vector<std::pair<int /*ref*/, Handler>> handlers;
        };

        inline static std::unordered_map<std::string, EventEntry> buffer;
        inline static std::mutex buffer_mutex;

        // Serialized exec store (for emit_callback round-trips)
        inline static std::unordered_map<std::string, int /*lua ref*/> exec_store;
        inline static std::mutex exec_mutex;

        // ----------------------------------------------------------------
        // Helpers
        // ----------------------------------------------------------------
        static EventEntry& get_or_create(const std::string& name) {
            std::lock_guard lock(buffer_mutex);
            return buffer[name];
        }

        static std::string serialize_exec(Machine* vm, int index) {
            // Store the Lua function and return an opaque key
            lua_pushvalue(vm->get_state(), index);
            int ref = luaL_ref(vm->get_state(), LUA_REGISTRYINDEX);
            std::string key = fmt::format("exec:{}", ref);
            std::lock_guard lock(exec_mutex);
            exec_store[key] = ref;
            return key;
        }

        static void deserialize_exec(Machine* vm, const std::string& key) {
            std::lock_guard lock(exec_mutex);
            auto it = exec_store.find(key);
            if (it == exec_store.end()) return;
            luaL_unref(vm->get_state(), LUA_REGISTRYINDEX, it->second);
            exec_store.erase(it);
        }

        // Execute a single handler (emit mode)
        static void execute_handler(Machine* vm, Handler& h, const std::string& name,
                                    const Tool::Stack& payload, bool is_callback) {
            auto state = vm->get_state();
            lua_rawgeti(state, LUA_REGISTRYINDEX, h.exec_ref);

            // Push payload args onto Lua stack from Tool::Stack
            int n_args = static_cast<int>(payload.array.size());
            for (auto& v : payload.array) vm -> push_value(v);

            if (!is_callback) {
                if (h.async) {
                    // Wrap in a new thread
                    // Move function + args to a new coroutine and resume
                    auto* thread_vm = vm->create_thread();
                    lua_xmove(state, thread_vm->get_state(), 1 + n_args);
                    thread_vm->resume(n_args);
                    // thread_vm lifetime managed by its coroutine; clean on finish
                } else {
                    lua_pcall(state, n_args, 0, 0);
                }
                if (h.subscription_limit > 0) {
                    h.subscription_count++;
                }
            } else {
                // callback mode: result goes back as a promise resolve
                // caller handles separately via emit_callback logic
                lua_pcall(state, n_args, LUA_MULTRET, 0);
            }
        }

        // ----------------------------------------------------------------
        // Bind
        // ----------------------------------------------------------------
        static void bind(Machine* vm) {
            // network.on(name, exec [, config]) -> bool
            API::bind(vm, {base_name}, "on", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, exec [, config])")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_function);

                std::string name = vm->get_string(1);
                bool is_priority = false, is_async = false, is_callback = false;
                int  sub_limit   = 0;

                if (vm->get_count() >= 3 && vm->is_table(3)) {
                    lua_getfield(vm->get_state(), 3, "isPrioritized");
                    is_priority = lua_toboolean(vm->get_state(), -1); lua_pop(vm->get_state(), 1);
                    lua_getfield(vm->get_state(), 3, "async");
                    is_async    = lua_toboolean(vm->get_state(), -1); lua_pop(vm->get_state(), 1);
                    lua_getfield(vm->get_state(), 3, "isCallback");
                    is_callback = lua_toboolean(vm->get_state(), -1); lua_pop(vm->get_state(), 1);
                    lua_getfield(vm->get_state(), 3, "subscriptionLimit");
                    if (lua_isnumber(vm->get_state(), -1))
                        sub_limit = std::max(1, (int)lua_tointeger(vm->get_state(), -1));
                    lua_pop(vm->get_state(), 1);
                }

                // Store Lua function reference
                lua_pushvalue(vm->get_state(), 2);
                int ref = luaL_ref(vm->get_state(), LUA_REGISTRYINDEX);

                Handler h;
                h.exec_ref          = ref;
                h.async             = is_async;
                h.is_callback       = is_callback;
                h.subscription_limit = sub_limit;
                h.subscription_count = 0;

                auto& entry = get_or_create(name);
                {
                    std::lock_guard lock(buffer_mutex);
                    if (is_callback) {
                        entry.callback_handler = h;
                    } else if (is_priority) {
                        entry.priority_handlers.emplace_back(ref, h);
                    } else {
                        entry.handlers.emplace_back(ref, h);
                    }
                }

                vm->push_value(true);
                return 1;
            });

            // network.off(name, exec) -> bool
            API::bind(vm, {base_name}, "off", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, exec)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_function);

                std::string name = vm->get_string(1);
                lua_pushvalue(vm->get_state(), 2);
                int ref = luaL_ref(vm->get_state(), LUA_REGISTRYINDEX); // temp ref to compare
                bool removed = false;

                std::lock_guard lock(buffer_mutex);
                auto it = buffer.find(name);
                if (it != buffer.end()) {
                    auto& entry = it->second;
                    auto erase_by_ref = [&](auto& vec) {
                        for (auto vit = vec.begin(); vit != vec.end(); ++vit) {
                            if (vit->first == ref) {
                                luaL_unref(vm->get_state(), LUA_REGISTRYINDEX, vit->second.exec_ref);
                                vec.erase(vit);
                                removed = true;
                                return;
                            }
                        }
                    };
                    erase_by_ref(entry.priority_handlers);
                    erase_by_ref(entry.handlers);
                    if (entry.callback_handler && entry.callback_handler->exec_ref == ref) {
                        luaL_unref(vm->get_state(), LUA_REGISTRYINDEX, entry.callback_handler->exec_ref);
                        entry.callback_handler.reset();
                        removed = true;
                    }
                }
                luaL_unref(vm->get_state(), LUA_REGISTRYINDEX, ref);
                vm->push_value(removed);
                return 1;
            });

            // network.emit(name, ...) -> bool
            // Fires all non-callback handlers for the event
            API::bind(vm, {base_name}, "emit", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, ...)")
                    .require(1, &Machine::is_string);

                std::string name = vm->get_string(1);

                // Collect variadic args into a Tool::Stack
                Tool::Stack payload;
                int top = vm->get_count();
                for (int i = 2; i <= top; ++i)
                    payload.array.push_back(Tool::Stack::read_value(vm, i));

                std::vector<std::pair<int, Handler>> to_fire_priority, to_fire_normal;
                {
                    std::lock_guard lock(buffer_mutex);
                    auto it = buffer.find(name);
                    if (it != buffer.end()) {
                        to_fire_priority = it->second.priority_handlers;
                        to_fire_normal   = it->second.handlers;
                    }
                }

                auto fire = [&](std::vector<std::pair<int, Handler>>& vec) {
                    std::vector<int> to_remove;
                    for (auto& [ref, h] : vec) {
                        if (h.is_callback) continue;
                        execute_handler(vm, h, name, payload, false);
                        if (h.subscription_limit > 0 && h.subscription_count >= h.subscription_limit)
                            to_remove.push_back(ref);
                    }
                    // Unsubscribe exhausted handlers
                    if (!to_remove.empty()) {
                        std::lock_guard lock(buffer_mutex);
                        auto it = buffer.find(name);
                        if (it != buffer.end()) {
                            auto& entry = it->second;
                            for (int r : to_remove) {
                                auto erase = [&](auto& v) {
                                    v.erase(std::remove_if(v.begin(), v.end(),
                                        [r](auto& p){ return p.first == r; }), v.end());
                                };
                                erase(entry.priority_handlers);
                                erase(entry.handlers);
                            }
                        }
                    }
                };

                fire(to_fire_priority);
                fire(to_fire_normal);

                vm->push_value(true);
                return 1;
            });

            // network.emit_callback(name, ...) -> Promise
            // Fires the single callback handler; returns a Promise the caller can await
            API::bind(vm, {base_name}, "emit_callback", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, ...)")
                    .require(1, &Machine::is_string);

                std::string name = vm->get_string(1);

                // Collect args
                Tool::Stack payload;
                int top = vm->get_count();
                for (int i = 2; i <= top; ++i)
                    payload.array.push_back(Tool::Stack::read_value(vm, i));

                // Create the promise that will be returned to Lua
                auto promise = Promise::make(vm);  // pushes userdata; stack top = promise

                std::optional<Handler> h_copy;
                {
                    std::lock_guard lock(buffer_mutex);
                    auto it = buffer.find(name);
                    if (it != buffer.end() && it->second.callback_handler)
                        h_copy = it->second.callback_handler;
                }

                if (!h_copy) {
                    // No callback registered — resolve immediately with no values
                    Promise::settle(promise, Promise::State::Resolved, vm, 0, 0);
                    return 1;  // promise is already on stack
                }

                auto& h = *h_copy;
                auto state = vm->get_state();

                // Push the handler function
                lua_rawgeti(state, LUA_REGISTRYINDEX, h.exec_ref);

                int n_args = static_cast<int>(payload.array.size());
                for (auto& v : payload.array) vm -> push_value(v);

                auto do_call = [&]() {
                    int base = lua_gettop(state) - n_args - 1;  // before fn
                    if (lua_pcall(state, n_args, LUA_MULTRET, 0) != LUA_OK) {
                        // Reject promise with error
                        int err_idx = lua_gettop(state);
                        Promise::settle(promise, Promise::State::Rejected, vm, err_idx, 1);
                        lua_pop(state, 1);
                    } else {
                        int result_count = lua_gettop(state) - base;
                        int args_start   = base + 1;
                        Promise::settle(promise, Promise::State::Resolved, vm, args_start, result_count);
                        lua_pop(state, result_count);
                    }
                };

                if (h.async) {
                    // Run in a new Lua coroutine; resolve promise when it finishes
                    auto* thread_vm = vm->create_thread();
                    lua_xmove(state, thread_vm->get_state(), 1 + n_args);
                    // Capture promise weakly so we can settle it after coroutine completes
                    auto weak_promise = std::weak_ptr<Promise::Instance>(promise);
                    thread_vm->on_finish = [vm, weak_promise, name](Machine* t, bool ok, int nret) {
                        auto p = weak_promise.lock();
                        if (!p) return;
                        if (ok) Promise::settle(p, Promise::State::Resolved,  t, lua_gettop(t->get_state()) - nret + 1, nret);
                        else    Promise::settle(p, Promise::State::Rejected,   t, lua_gettop(t->get_state()),              1);
                    };
                    thread_vm->resume(n_args);
                } else {
                    do_call();
                }

                // Promise userdata already on the stack from Promise::make
                return 1;
            });
        }

        static void clean(const std::string& env) {
            std::lock_guard lock(buffer_mutex);
            // Release all Lua refs — called on VM teardown
            buffer.erase(env);
        }
    };
}