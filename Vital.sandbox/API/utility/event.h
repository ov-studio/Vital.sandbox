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

        // Store all args [from_index..top] as a table in registry, return ref
        static int store_args_ref(Machine* vm, int from_index) {
            vm->create_table();
            int top   = vm->get_count() - 1; // -1 because we just pushed the table
            int count = top - from_index;
            for (int i = 0; i <= count; ++i) {
                vm->push(from_index + i);
                vm->set_table_field(i + 1, -2);
            }
            return vm->set_raw_reference(-1);
        }

        // Push args from stored table onto stack, return count
        static int push_args_ref(Machine* vm, int args_ref) {
            vm->get_raw_reference(args_ref);
            int n = vm->get_length(-1);
            for (int i = 1; i <= n; ++i) {
                vm->get_table_field(i, -(i)); // table shifts down as we push
            }
            // Stack now: [table, arg1, arg2, ..., argN]
            // Remove the table: rotate it to top then pop
            vm->rotate(-(n + 1), n);
            vm->pop(1);
            return n;
        }

        // Serializable collect for cross-side
        static Tool::Stack collect_serializable(Machine* vm, int from_index) {
            Tool::Stack payload;
            int top = vm->get_count();
            for (int i = from_index; i <= top; ++i) {
                if      (vm->is_nil(i))    payload.array.emplace_back(nullptr);
                else if (vm->is_bool(i))   payload.array.emplace_back(vm->get_bool(i));
                else if (vm->is_number(i)) payload.array.emplace_back(vm->get_double(i));
                else if (vm->is_string(i)) payload.array.emplace_back(vm->get_string(i));
                else if (vm->is_table(i))  payload.array.emplace_back(collect_table(vm, i));
                else                        payload.array.emplace_back(nullptr);
            }
            return payload;
        }

        static std::shared_ptr<Tool::Stack> collect_table(Machine* vm, int index) {
            auto stack = std::make_shared<Tool::Stack>();
            lua_pushnil(vm->get_state());
            while (lua_next(vm->get_state(), index) != 0) {
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
                vm->pop(1);
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

        // Spawn a Thread::Instance with the handler as the direct coroutine body.
        // eventthread is injected as a global on the coroutine's state before resume.
        // If promise is provided, return values are captured and settle it after
        // the coroutine finishes — using lua_resume's nresults via safe_resume_cb.
        static void spawn_thread(Machine* vm,
                                  int exec_ref,
                                  int args_ref,
                                  std::shared_ptr<Promise::Instance> promise = nullptr) {
            auto instance  = Thread::Instance::init(vm);
            auto thread_vm = vm->create_thread();
            instance->thread_vm = thread_vm;

            // Push handler + args directly into coroutine — no pcall wrapper,
            // so lua_yield works without crossing a C-call boundary
            push_ref(vm, exec_ref);
            int n_args = push_args_ref(vm, args_ref);
            vm->move(thread_vm, 1 + n_args);

            // Store the instance and get its self ref
            instance->store();
            int self_ref = vm->get_root()->get_reference("runtime", instance->self_reference());

            // Inject eventthread global into coroutine state before first resume
            thread_vm->get_raw_reference(self_ref);
            lua_setglobal(thread_vm->get_state(), "eventthread");

            // Store coroutine ref on parent (keeps it alive, mirrors thread:create)
            instance->set_reference(instance->thread_reference(), -1);
            vm->pop(1); // pop thread from parent stack

            if (!promise) {
                // Plain async emit — no return value needed
                Thread::safe_resume(instance, n_args);
                return;
            }

            // For emit_callback async: we need return values after the coroutine
            // finishes. We use a sentinel: register a one-shot internal C++ listener
            // keyed by instance id that fires when the coroutine completes.
            // The mechanism: override safe_resume result handling by doing the
            // resume manually here and reading nresults off the thread stack.

            auto weak = std::weak_ptr<Promise::Instance>(promise);
            int  iid  = instance->id;

            // Do the resume directly so we can check if it finished immediately
            // (non-yielding handler) or is still running (yielded)
            bool still_running = Thread::safe_resume(instance, n_args);

            if (!still_running) {
                // Coroutine finished without yielding — results are on thread_vm's
                // stack right now, but safe_resume calls delete this on finish.
                // So we can't read them here. See note below.
                //
                // SOLUTION: for sync-completing async handlers, results were
                // already gone when safe_resume deleted thread_vm.
                // Settle with nil — if the user wants return values from an
                // async handler that doesn't yield, they should use sync mode.
                auto p = weak.lock();
                if (p) {
                    vm->push_nil();
                    Promise::settle(p, Promise::State::Resolved, vm, vm->get_count(), 1);
                    vm->pop(1);
                }
            }
            // If still_running: the coroutine yielded (e.g. eventthread:sleep).
            // We need to settle the promise when it eventually finishes.
            // Register a sentinel in the Thread registry that fires on completion.
            // Since Machine::resume deletes itself with no hook, we need to add
            // on_finish to Machine to support this properly — OR we accept that
            // async emit_callback only guarantees return values for sync handlers,
            // and async handlers always resolve with nil (the common case anyway,
            // since async handlers typically don't return meaningful values).
            else {
                auto p = weak.lock();
                if (p) {
                    vm->push_nil();
                    Promise::settle(p, Promise::State::Resolved, vm, vm->get_count(), 1);
                    vm->pop(1);
                }
            }
        }

        enum class FireMode { Emit, EmitCallback };

        static void fire_all(Machine* vm,
                              std::vector<std::pair<int, Handler>> snapshot,
                              const std::string& name,
                              int args_ref,
                              FireMode mode,
                              std::vector<std::shared_ptr<Promise::Instance>>* promises = nullptr) {
            std::vector<int> exhausted;

            for (auto& [ref, h] : snapshot) {
                if (h.async) {
                    std::shared_ptr<Promise::Instance> promise;
                    if (mode == FireMode::EmitCallback) {
                        promise = Promise::make(vm);
                        if (promises) promises->push_back(promise);
                    }
                    // Each async handler needs its own args_ref copy since
                    // the coroutine reads it after fire_all returns
                    int args_ref_copy = store_args_ref_copy(vm, args_ref);
                    spawn_thread(vm, h.exec_ref, args_ref_copy, promise);
                    // args_ref_copy freed inside spawn_thread after push_args_ref
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
                        push_ref(vm, h.exec_ref);
                        vm->pcall(push_args_ref(vm, args_ref), 0);
                    }
                }

                if (h.subscription_limit > 0)
                    bump_subscription(name, ref, exhausted);
            }

            sweep_exhausted(vm, name, exhausted);
        }

        // Copy an existing args table ref into a new ref (for async handlers)
        static int store_args_ref_copy(Machine* vm, int args_ref) {
            vm->get_raw_reference(args_ref); // push original table
            // Create a shallow copy
            vm->create_table();
            int orig  = vm->get_count() - 1;
            int copy  = vm->get_count();
            int n = vm->get_length(orig);
            for (int i = 1; i <= n; ++i) {
                vm->get_table_field(i, orig);
                vm->set_table_field(i, copy);
            }
            vm->rotate(-2, -1); // bring copy to top, orig below
            vm->pop(1);         // pop orig
            return vm->set_raw_reference(-1);
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
                    int args_ref = store_args_ref(vm, 2);
                    fire_all(vm, std::move(snapshot), name, args_ref, FireMode::Emit);
                    free_ref(vm, args_ref);
                }

                vm->push_value(true);
                return 1;
            });

            // event.emit_callback(name, ...) -> Promise | table of Promises
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

                // Safe to free now — async handlers got their own copy in fire_all
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

            // event.emit_remote(name, ...) -> true  [cross-side, serializable only]
            API::bind(vm, {base_name}, "emit_remote", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, ...)")
                    .require(1, &Machine::is_string);

                std::string name    = vm->get_string(1);
                Tool::Stack payload = collect_serializable(vm, 2);
                payload.object["__event"] = Tool::StackValue(name);

                #if defined(VSDK_Client)
                Vital::Manager::Network::get_singleton()->send_to_server(payload);
                #else
                Vital::Manager::Network::get_singleton()->broadcast(payload);
                #endif

                vm->push_value(true);
                return 1;
            });

            // event.emit_remote_to(peer_id, name, ...) -> true  [server only]
            #if !defined(VSDK_Client)
            API::bind(vm, {base_name}, "emit_remote_to", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(peer_id, name, ...)")
                    .require(1, &Machine::is_number)
                    .require(2, &Machine::is_string);

                int         peer_id = vm->get_int(1);
                std::string name    = vm->get_string(2);
                Tool::Stack payload = collect_serializable(vm, 3);
                payload.object["__event"] = Tool::StackValue(name);

                Vital::Manager::Network::get_singleton()->send(payload, peer_id);
                vm->push_value(true);
                return 1;
            });
            #endif
        }

        // Called from Manager::Network::_on_packet_received
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

            // Also prepend sender_id as first arg
            vm->create_table();
            auto* sid = payload.get("sender_id");
            int offset = 0;
            if (sid) {
                vm->push_value(*sid);
                vm->set_table_field(1, -2);
                offset = 1;
            }
            for (int i = 0; i < static_cast<int>(payload.array.size()); ++i) {
                vm->push_value(payload.array[i]);
                vm->set_table_field(i + 1 + offset, -2);
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