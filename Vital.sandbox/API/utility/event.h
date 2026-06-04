/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: event.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Event APIs
----------------------------------------------------------------*/

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Manager/public/network.h>
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/API/utility/promise.h>
#include <Vital.sandbox/API/utility/thread.h>


////////////////////////
// Vital: API: Event //
////////////////////////

namespace Vital::Sandbox::API {
    struct Event : vm_module {
        inline static const std::string base_name = "event";

        struct Handler {
            int  exec_ref           = LUA_NOREF;
            bool async              = false;
            int  subscription_limit = 0;
            int  subscription_count = 0;
        };

        struct EventEntry {
            std::vector<std::pair<int, Handler>> handlers;
        };

        inline static std::unordered_map<std::string, EventEntry> buffer;
        inline static std::mutex buffer_mutex;


        // ----------------------------------------------------------------
        // Helpers
        // ----------------------------------------------------------------

        static int store_ref(Machine* vm, int index) {
            vm->push(index);
            return vm->set_raw_reference(-1);
        }

        static void free_ref(Machine* vm, int ref) {
            if (ref != LUA_NOREF) vm->del_raw_reference(ref);
        }

        static void push_ref(Machine* vm, int ref) {
            vm->get_raw_reference(ref);
        }

        struct HandlerConfig {
            bool is_async  = false;
            int  sub_limit = 0;
        };

        static HandlerConfig read_config(Machine* vm, int index) {
            HandlerConfig cfg;
            if (vm->get_count() < index || !vm->is_table(index)) return cfg;
            vm->get_table_field("async",             index); cfg.is_async  = vm->get_bool(-1); vm->pop(1);
            vm->get_table_field("subscriptionLimit", index);
            if (vm->is_number(-1)) cfg.sub_limit = std::max(1, vm->get_int(-1));
            vm->pop(1);
            return cfg;
        }

        // Same-side: pass raw Lua values by reference via registry
        // Cross-side: only serializable values (nil, bool, number, string, table of those)
        struct ArgRef {
            int  ref       = LUA_NOREF;  // registry ref for same-side
            bool is_remote = false;
        };

        // Store all args as a single table in the registry for same-side passing
        static int store_args_ref(Machine* vm, int from_index) {
            vm->create_table();
            int top = vm->get_count() - 1; // table is at top, args below
            int count = top - from_index;
            for (int i = 0; i <= count; ++i) {
                vm->push(from_index + i);
                vm->set_table_field(i + 1, -2);
            }
            return vm->set_raw_reference(-1);
        }

        // Push stored args back onto stack, returns count
        static int push_args_ref(Machine* vm, int args_ref) {
            vm->get_raw_reference(args_ref);        // push the table
            int n = vm->get_length(-1);
            for (int i = 1; i <= n; ++i)
                vm->get_table_field(i, -1 - (i - 1)); // push each field, table shifts
            vm->rotate(-(n + 1), n);                // bring table to top
            vm->pop(1);                             // pop the table
            return n;
        }

        // Serializable-only collect for cross-side (Tool::Stack)
        static Tool::Stack collect_serializable(Machine* vm, int from_index) {
            Tool::Stack payload;
            int top = vm->get_count();
            for (int i = from_index; i <= top; ++i) {
                if      (vm->is_nil(i))    payload.array.emplace_back(nullptr);
                else if (vm->is_bool(i))   payload.array.emplace_back(vm->get_bool(i));
                else if (vm->is_number(i)) payload.array.emplace_back(vm->get_double(i));
                else if (vm->is_string(i)) payload.array.emplace_back(vm->get_string(i));
                else if (vm->is_table(i))  payload.array.emplace_back(collect_table(vm, i));
                else                        payload.array.emplace_back(nullptr); // non-serializable becomes nil
            }
            return payload;
        }

        static std::shared_ptr<Tool::Stack> collect_table(Machine* vm, int index) {
            auto stack = std::make_shared<Tool::Stack>();
            lua_pushnil(vm->get_state());
            while (lua_next(vm->get_state(), index) != 0) {
                // key at -2, value at -1
                // only string keys for object, integer keys for array
                if (vm->is_string(-2)) {
                    std::string key = vm->get_string(-2);
                    if      (vm->is_nil(-1))    stack->object[key] = Tool::StackValue(nullptr);
                    else if (vm->is_bool(-1))   stack->object[key] = Tool::StackValue(vm->get_bool(-1));
                    else if (vm->is_number(-1)) stack->object[key] = Tool::StackValue(vm->get_double(-1));
                    else if (vm->is_string(-1)) stack->object[key] = Tool::StackValue(vm->get_string(-1));
                    else if (vm->is_table(-1))  stack->object[key] = Tool::StackValue(collect_table(vm, vm->get_count()));
                } else if (vm->is_number(-2)) {
                    int idx = vm->get_int(-2);
                    Tool::StackValue val;
                    if      (vm->is_nil(-1))    val = Tool::StackValue(nullptr);
                    else if (vm->is_bool(-1))   val = Tool::StackValue(vm->get_bool(-1));
                    else if (vm->is_number(-1)) val = Tool::StackValue(vm->get_double(-1));
                    else if (vm->is_string(-1)) val = Tool::StackValue(vm->get_string(-1));
                    else if (vm->is_table(-1))  val = Tool::StackValue(collect_table(vm, vm->get_count()));
                    if (idx >= 1) {
                        if (static_cast<int>(stack->array.size()) < idx)
                            stack->array.resize(idx, Tool::StackValue(nullptr));
                        stack->array[idx - 1] = val;
                    }
                }
                vm->pop(1); // pop value, keep key
            }
            return stack;
        }

        static void bump_subscription(const std::string& name, int ref, std::vector<int>& exhausted) {
            std::lock_guard lock(buffer_mutex);
            auto it = buffer.find(name);
            if (it == buffer.end()) return;
            for (auto& [vref, vh] : it->second.handlers) {
                if (vref != ref) continue;
                vh.subscription_count++;
                if (vh.subscription_count >= vh.subscription_limit)
                    exhausted.push_back(ref);
                return;
            }
        }

        static void sweep_exhausted(Machine* vm, const std::string& name, const std::vector<int>& dead_refs) {
            if (dead_refs.empty()) return;
            std::lock_guard lock(buffer_mutex);
            auto it = buffer.find(name);
            if (it == buffer.end()) return;
            for (auto vit = it->second.handlers.begin(); vit != it->second.handlers.end(); ) {
                if (std::find(dead_refs.begin(), dead_refs.end(), vit->first) != dead_refs.end()) {
                    free_ref(vm, vit->second.exec_ref);
                    vit = it->second.handlers.erase(vit);
                } else ++vit;
            }
        }

        // Create a Thread::Instance, inject `eventthread` local into the coroutine,
        // push handler + args, and resume.
        // `promise` is optional — if provided, return values settle it.
        static void spawn_thread(Machine* vm,
                                  int exec_ref,
                                  int args_ref,          // registry ref to args table
                                  std::shared_ptr<Promise::Instance> promise = nullptr) {
            auto instance  = Thread::Instance::init(vm);
            auto thread_vm = vm->create_thread();
            instance->thread_vm = thread_vm;

            // We push a driver closure that:
            //   1. Injects `eventthread` as an upvalue-accessible local
            //   2. Calls the real handler with the stored args
            //   3. If promise exists, settles it with return values

            struct DriverCtx {
                int                                exec_ref;
                int                                args_ref;
                int                                instance_self_ref; // Thread::Instance userdata ref
                std::weak_ptr<Promise::Instance>   promise;
            };

            // Store the thread instance userdata ref so the closure can push it
            // We need to store the instance first so we have a self_reference
            instance->store();

            // Grab the self ref the store() just created
            int self_ref = vm->get_root()->get_reference("runtime", instance->self_reference());

            auto* ctx = new DriverCtx { exec_ref, args_ref, self_ref,
                                         promise ? std::weak_ptr<Promise::Instance>(promise)
                                                 : std::weak_ptr<Promise::Instance>() };

            lua_pushlightuserdata(vm->get_state(), ctx);
            lua_pushcclosure(vm->get_state(), [](lua_State* state) -> int {
                auto* ctx      = static_cast<DriverCtx*>(lua_touserdata(state, lua_upvalueindex(1)));
                auto* inner_vm = Machine::fetch_machine(state);

                // Inject eventthread = the Thread::Instance userdata
                inner_vm->get_raw_reference(ctx->instance_self_ref);
                lua_setglobal(state, "eventthread"); // set as global in this coroutine's env

                // Push handler and args
                inner_vm->get_raw_reference(ctx->exec_ref);
                int n_args = push_args_ref(inner_vm, ctx->args_ref);

                // Free the args ref now that we've pushed them
                inner_vm->del_raw_reference(ctx->args_ref);

                int base = inner_vm->get_count() - n_args - 1;
                bool ok  = inner_vm->pcall(n_args, LUA_MULTRET);

                auto p = ctx->promise.lock();
                delete ctx;

                if (p) {
                    if (ok) {
                        int result_count = inner_vm->get_count() - base;
                        if (result_count > 0) {
                            Promise::settle(p, Promise::State::Resolved, inner_vm, base + 1, result_count);
                            inner_vm->pop(result_count);
                        } else {
                            inner_vm->push_nil();
                            Promise::settle(p, Promise::State::Resolved, inner_vm, inner_vm->get_count(), 1);
                            inner_vm->pop(1);
                        }
                    } else {
                        Promise::settle(p, Promise::State::Rejected, inner_vm, 0, 0);
                    }
                }
                return 0;
            }, 1);

            // Move the closure into the coroutine
            vm->move(thread_vm, 1);

            // Store thread coroutine ref (mirrors thread:create)
            instance->set_reference(instance->thread_reference(), -1);
            vm->pop(1); // pop coroutine from parent stack

            Thread::safe_resume(instance, 0);
        }

        enum class FireMode { Emit, EmitCallback };

        static void fire_all(Machine* vm,
                              std::vector<std::pair<int, Handler>> snapshot,
                              const std::string& name,
                              int args_ref,             // registry ref to args table (same-side)
                              FireMode mode,
                              std::vector<std::shared_ptr<Promise::Instance>>* promises = nullptr) {
            std::vector<int> exhausted;

            for (auto& [ref, h] : snapshot) {
                if (h.async) {
                    // Always run async handlers in a Thread::Instance
                    // so sleep/await/pause work, and eventthread is injected
                    std::shared_ptr<Promise::Instance> promise;
                    if (mode == FireMode::EmitCallback) {
                        promise = Promise::make(vm);
                        if (promises) promises->push_back(promise);
                    }
                    spawn_thread(vm, h.exec_ref, args_ref, promise);
                } else {
                    if (mode == FireMode::EmitCallback) {
                        auto promise = Promise::make(vm);
                        if (promises) promises->push_back(promise);

                        push_ref(vm, h.exec_ref);
                        int n_args = push_args_ref(vm, args_ref);
                        int base   = vm->get_count() - n_args - 1;

                        if (vm->pcall(n_args, LUA_MULTRET)) {
                            int result_count = vm->get_count() - base;
                            if (result_count > 0) {
                                Promise::settle(promise, Promise::State::Resolved, vm, base + 1, result_count);
                            } else {
                                vm->push_nil();
                                Promise::settle(promise, Promise::State::Resolved, vm, vm->get_count(), 1);
                                vm->pop(1);
                            }
                            vm->pop(result_count);
                        } else {
                            Promise::settle(promise, Promise::State::Rejected, vm, 0, 0);
                        }
                    } else {
                        // Emit sync: fire and forget
                        push_ref(vm, h.exec_ref);
                        int n_args = push_args_ref(vm, args_ref);
                        vm->pcall(n_args, 0);
                    }
                }

                if (h.subscription_limit > 0)
                    bump_subscription(name, ref, exhausted);
            }

            sweep_exhausted(vm, name, exhausted);
        }


        // ----------------------------------------------------------------
        // Bind
        // ----------------------------------------------------------------
        static void bind(Machine* vm) {

            // event.on(name, exec [, config]) -> true
            API::bind(vm, {base_name}, "on", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, exec [, config])")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_function);

                std::string   name = vm->get_string(1);
                HandlerConfig cfg  = read_config(vm, 3);

                Handler h;
                h.exec_ref           = store_ref(vm, 2);
                h.async              = cfg.is_async;
                h.subscription_limit = cfg.sub_limit;
                h.subscription_count = 0;

                {
                    std::lock_guard lock(buffer_mutex);
                    buffer[name].handlers.emplace_back(h.exec_ref, h);
                }

                vm->push_value(true);
                return 1;
            });

            // event.off(name, exec) -> bool
            API::bind(vm, {base_name}, "off", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, exec)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_function);

                std::string name = vm->get_string(1);
                bool removed = false;

                vm->push(2);
                int lookup_ref = vm->set_raw_reference(-1);

                {
                    std::lock_guard lock(buffer_mutex);
                    auto it = buffer.find(name);
                    if (it != buffer.end()) {
                        auto& vec = it->second.handlers;
                        for (auto vit = vec.begin(); vit != vec.end(); ++vit) {
                            vm->get_raw_reference(lookup_ref);
                            vm->get_raw_reference(vit->second.exec_ref);
                            bool eq = lua_rawequal(vm->get_state(), -1, -2);
                            vm->pop(2);
                            if (!eq) continue;
                            free_ref(vm, vit->second.exec_ref);
                            vec.erase(vit);
                            removed = true;
                            break;
                        }
                    }
                }

                free_ref(vm, lookup_ref);
                vm->push_value(removed);
                return 1;
            });

            // event.emit(name, ...) -> true
            // Same-side: args passed by reference, any Lua value allowed
            API::bind(vm, {base_name}, "emit", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, ...)")
                    .require(1, &Machine::is_string);

                std::string name = vm->get_string(1);

                std::vector<std::pair<int, Handler>> snapshot;
                {
                    std::lock_guard lock(buffer_mutex);
                    auto it = buffer.find(name);
                    if (it != buffer.end()) snapshot = it->second.handlers;
                }

                if (!snapshot.empty()) {
                    // Store args by ref — all handlers share the same arg table
                    int args_ref = store_args_ref(vm, 2);
                    fire_all(vm, std::move(snapshot), name, args_ref, FireMode::Emit);
                    free_ref(vm, args_ref);
                }

                vm->push_value(true);
                return 1;
            });

            // event.emit_callback(name, ...) -> Promise | table of Promises
            // Same-side: args passed by reference
            API::bind(vm, {base_name}, "emit_callback", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, ...)")
                    .require(1, &Machine::is_string);

                std::string name = vm->get_string(1);

                std::vector<std::pair<int, Handler>> snapshot;
                {
                    std::lock_guard lock(buffer_mutex);
                    auto it = buffer.find(name);
                    if (it != buffer.end()) snapshot = it->second.handlers;
                }

                if (snapshot.empty()) {
                    auto promise = Promise::make(vm);
                    vm->push_nil();
                    Promise::settle(promise, Promise::State::Resolved, vm, vm->get_count(), 1);
                    vm->pop(1);
                    return 1;
                }

                int args_ref = store_args_ref(vm, 2);

                std::vector<std::shared_ptr<Promise::Instance>> promises;
                fire_all(vm, std::move(snapshot), name, args_ref, FireMode::EmitCallback, &promises);

                free_ref(vm, args_ref);

                if (promises.size() == 1) {
                    promises[0]->get_reference(promises[0]->self_reference(), true);
                } else {
                    vm->create_table();
                    for (int i = 0; i < static_cast<int>(promises.size()); ++i) {
                        promises[i]->get_reference(promises[i]->self_reference(), true);
                        vm->set_table_field(i + 1, -2);
                    }
                }

                return 1;
            });

            // event.emit_remote(name, payload) -> true
            // Cross-side emit: serializable values only, goes through Manager::Network
            API::bind(vm, {base_name}, "emit_remote", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, ...)")
                    .require(1, &Machine::is_string);

                std::string name    = vm->get_string(1);
                Tool::Stack payload = collect_serializable(vm, 2);
                payload.object["__event"] = Tool::StackValue(name);

                #if defined(VSDK_Client)
                Manager::Network::get_singleton()->send_to_server(payload);
                #else
                // emit_remote on server broadcasts to all peers
                Manager::Network::get_singleton()->broadcast(payload);
                #endif

                vm->push_value(true);
                return 1;
            });

            // event.emit_remote_to(peer_id, name, ...) -> true   [server only]
            #if !defined(VSDK_Client)
            API::bind(vm, {base_name}, "emit_remote_to", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(peer_id, name, ...)")
                    .require(1, &Machine::is_number)
                    .require(2, &Machine::is_string);

                int         peer_id = vm->get_int(1);
                std::string name    = vm->get_string(2);
                Tool::Stack payload = collect_serializable(vm, 3);
                payload.object["__event"] = Tool::StackValue(name);

                Manager::Network::get_singleton()->send(payload, peer_id);
                vm->push_value(true);
                return 1;
            });
            #endif
        }

        // Called from Manager::Network::_on_packet_received to dispatch
        // incoming remote events into the local event system
        static void dispatch_remote(Machine* vm, const Tool::Stack& payload) {
            auto event_name_ptr = payload.get("__event");
            if (!event_name_ptr || !event_name_ptr->is<std::string>()) return;
            std::string name = event_name_ptr->as<std::string>();

            std::vector<std::pair<int, Handler>> snapshot;
            {
                std::lock_guard lock(buffer_mutex);
                auto it = buffer.find(name);
                if (it != buffer.end()) snapshot = it->second.handlers;
            }
            if (snapshot.empty()) return;

            // Push serializable payload as Lua args, store by ref, fire
            vm->create_table();
            for (int i = 0; i < static_cast<int>(payload.array.size()); ++i) {
                vm->push_value(payload.array[i]);
                vm->set_table_field(i + 1, -2);
            }
            int args_ref = vm->set_raw_reference(-1);

            fire_all(vm, std::move(snapshot), name, args_ref, FireMode::Emit);
            free_ref(vm, args_ref);
        }

        static void clean(const std::string& env) {
            std::lock_guard lock(buffer_mutex);
            buffer.erase(env);
        }
    };
}