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
#include <Vital.sandbox/Manager/public/network.h>
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/API/utility/promise.h>
#include <Vital.sandbox/API/utility/thread.h>


////////////////////////
// Vital: API: Event //
////////////////////////

namespace Vital::Sandbox::API {
    // TODO: Improve
    struct Event : vm_module {
        inline static const std::string base_name = "event";

        struct Handler {
            int         exec_ref           = LUA_NOREF;
            bool        async              = false;
            int         subscription_limit = 0;
            int         subscription_count = 0;
            std::string env;
        };

        struct EventEntry {
            std::vector<std::pair<int, Handler>> handlers;
        };

        inline static std::unordered_map<std::string, EventEntry> buffer;
        inline static std::mutex buffer_mutex;

        // serial -> Promise for pending remote emit_callback calls
        inline static std::unordered_map<uint32_t, std::shared_ptr<Promise::Instance>> pending_remote;
        inline static std::mutex pending_remote_mutex;
        inline static std::atomic<uint32_t> serial_counter { 0 };

        // promise_id -> reply callback for remote emit_callback response
        using ReplyCallback = std::function<void(Machine*, const Tool::Stack&)>;
        inline static std::unordered_map<int, ReplyCallback> reply_callbacks;
        inline static std::mutex reply_callbacks_mutex;


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

        static int store_args_ref(Machine* vm, int from_index) {
            vm->create_table();
            int top   = vm->get_count() - 1;
            int count = top - from_index;
            for (int i = 0; i <= count; ++i) {
                vm->push(from_index + i);
                vm->set_table_field(i + 1, -2);
            }
            return vm->set_raw_reference(-1);
        }

        static int push_args_ref(Machine* vm, int args_ref) {
            vm->get_raw_reference(args_ref);
            int table_idx = vm->get_count();
            int n         = vm->get_length(table_idx);
            for (int i = 1; i <= n; ++i)
                vm->get_table_field(i, table_idx);
            vm->rotate(table_idx, -1);
            vm->pop(1);
            return n;
        }

        static int store_args_ref_copy(Machine* vm, int args_ref) {
            vm->get_raw_reference(args_ref);
            vm->create_table();
            int orig = vm->get_count() - 1;
            int copy = vm->get_count();
            int n    = vm->get_length(orig);
            for (int i = 1; i <= n; ++i) {
                vm->get_table_field(i, orig);
                vm->set_table_field(i, copy);
            }
            vm->rotate(-2, -1);
            vm->pop(1);
            return vm->set_raw_reference(-1);
        }

        static void push_promise(Machine* vm, std::shared_ptr<Promise::Instance> promise) {
            vm->get_raw_reference(promise->get_reference(promise->self_reference()));
        }

        // Send a packet to the appropriate destination.
        // On client: always sends to server (peer_id ignored).
        // On server: peer_id=0 broadcasts to all, peer_id>0 sends to that specific peer.
        static void send_packet(const Tool::Stack& payload, int peer_id = 0) {
            #if defined(VSDK_Client)
            Vital::Manager::Network::get_singleton()->send_to_server(payload);
            #else
            if (peer_id > 0) Vital::Manager::Network::get_singleton()->send(payload, peer_id);
            else             Vital::Manager::Network::get_singleton()->broadcast(payload);
            #endif
        }

        // Build and send a remote emit payload, optionally with a serial for callback.
        // Returns the promise (non-null only when serial > 0).
        static std::shared_ptr<Promise::Instance> send_remote_emit(Machine* vm,
                                                                     const std::string& name,
                                                                     Tool::Stack payload,
                                                                     int peer_id,
                                                                     bool wants_callback) {
            payload.object["__event"] = Tool::StackValue(name);

            std::shared_ptr<Promise::Instance> promise;
            if (wants_callback) {
                uint32_t serial = ++serial_counter;
                payload.object["__serial"] = Tool::StackValue(static_cast<double>(serial));
                promise = Promise::make(vm);
                vm->pop(1);
                {
                    std::lock_guard lock(pending_remote_mutex);
                    pending_remote[serial] = promise;
                }
            }

            send_packet(payload, peer_id);
            return promise;
        }

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
            auto* L = vm->get_state();
            lua_pushnil(L);
            while (lua_next(L, index) != 0) {
                // Use lua_type for the KEY — lua_isstring returns true for numbers too,
                // and calling lua_tostring/get_string on an integer key during iteration
                // coerces it in-place, making it invalid for the next lua_next call.
                int key_type = lua_type(L, -2);
                if (key_type == LUA_TNUMBER) {
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
                } else if (key_type == LUA_TSTRING) {
                    std::string key = vm->get_string(-2);
                    if      (vm->is_nil(-1))    stack->object[key] = Tool::StackValue(nullptr);
                    else if (vm->is_bool(-1))   stack->object[key] = Tool::StackValue(vm->get_bool(-1));
                    else if (vm->is_number(-1)) stack->object[key] = Tool::StackValue(vm->get_double(-1));
                    else if (vm->is_string(-1)) stack->object[key] = Tool::StackValue(vm->get_string(-1));
                    else if (vm->is_table(-1))  stack->object[key] = Tool::StackValue(collect_table(vm, vm->get_count()));
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

        // Spawn a Thread::Instance coroutine for an async event handler.
        // Always creates the coroutine under root_vm so its lifetime is independent
        // of whatever coroutine called spawn_thread.
        static void spawn_thread(Machine* vm,
                                  int exec_ref,
                                  int args_ref,
                                  std::shared_ptr<Promise::Instance> promise = nullptr) {
            auto* root_vm   = vm->get_root();
            auto  instance  = Thread::Instance::init(vm);
            auto  thread_vm = root_vm->create_thread();
            instance->thread_vm = thread_vm;

            int coroutine_idx = root_vm->get_count();

            push_ref(vm, exec_ref);
            int n_args = push_args_ref(vm, args_ref);
            vm->move(thread_vm, 1 + n_args);

            instance->store();
            int self_ref = root_vm->get_reference("runtime", instance->self_reference());

            thread_vm->get_raw_reference(self_ref);
            lua_setglobal(thread_vm->get_state(), "eventthread");

            instance->set_reference(instance->thread_reference(), coroutine_idx);
            root_vm->pop(1);
            vm->pop(1);

            if (promise) {
                auto weak_promise = std::weak_ptr<Promise::Instance>(promise);
                thread_vm->set_finish_hook([weak_promise, root_vm](Machine* thread_vm, int nresults) {
                    auto p = weak_promise.lock();
                    if (!p) return;
                    int base  = root_vm->get_count() + 1;
                    int count = nresults > 0 ? nresults : 0;
                    if (nresults > 0) lua_xmove(thread_vm->get_state(), root_vm->get_state(), nresults);
                    Promise::settle(p, Promise::State::Resolved, root_vm, base, count);
                    if (count > 0) root_vm->pop(count);
                });
            }

            Thread::safe_resume(instance, n_args);
        }

        enum class FireMode { Emit, EmitCallback };

        // Fire a single handler, creating and returning a promise when mode=EmitCallback.
        static std::shared_ptr<Promise::Instance> fire_one(Machine* vm,
                                                            const Handler& h,
                                                            int args_ref,
                                                            FireMode mode) {
            std::shared_ptr<Promise::Instance> promise;
            if (mode == FireMode::EmitCallback) {
                promise = Promise::make(vm);
                vm->pop(1);
            }

            if (h.async) {
                int args_ref_copy = store_args_ref_copy(vm, args_ref);
                spawn_thread(vm, h.exec_ref, args_ref_copy, promise);
                free_ref(vm, args_ref_copy);
            } else {
                push_ref(vm, h.exec_ref);
                int n_args = push_args_ref(vm, args_ref);
                if (mode == FireMode::EmitCallback) {
                    int base = vm->get_count() - n_args - 1;
                    if (vm->pcall(n_args, LUA_MULTRET)) {
                        int result_count = vm->get_count() - base;
                        Promise::settle(promise, Promise::State::Resolved, vm, base + 1, result_count);
                        vm->pop(result_count);
                    } else {
                        Promise::settle(promise, Promise::State::Rejected, vm, 0, 0);
                    }
                } else {
                    vm->pcall(n_args, 0);
                }
            }

            return promise;
        }

        static void fire_all(Machine* vm,
                              std::vector<std::pair<int, Handler>> snapshot,
                              const std::string& name,
                              int args_ref,
                              FireMode mode,
                              std::vector<std::shared_ptr<Promise::Instance>>* promises = nullptr) {
            std::vector<int> exhausted;

            for (auto& [ref, h] : snapshot) {
                auto promise = fire_one(vm, h, args_ref, mode);
                if (promise && promises) promises->push_back(promise);
                if (h.subscription_limit > 0) bump_subscription(name, ref, exhausted);
            }

            sweep_exhausted(vm, name, exhausted);
        }

        // Register a reply callback on a promise using sentinel thread_id=-1.
        // When the promise settles, Thread::reply_dispatcher routes to dispatch_reply.
        static void register_reply_callback(std::shared_ptr<Promise::Instance> promise,
                                             ReplyCallback cb) {
            {
                std::lock_guard lock(reply_callbacks_mutex);
                reply_callbacks[promise->id] = std::move(cb);
            }
            promise->waiting.push_back(-1);
        }

        // Called by Thread::reply_dispatcher when a promise with sentinel -1 settles.
        static void dispatch_reply(int promise_id, std::shared_ptr<Promise::Instance> promise) {
            ReplyCallback cb;
            {
                std::lock_guard lock(reply_callbacks_mutex);
                auto it = reply_callbacks.find(promise_id);
                if (it == reply_callbacks.end()) return;
                cb = std::move(it->second);
                reply_callbacks.erase(it);
            }

            auto* root_vm = Manager::Sandbox::get_singleton()->get_vm();
            if (!root_vm || !cb) return;

            Tool::Stack results;
            for (int i = 1; i <= promise->value_count; ++i) {
                root_vm->get_raw_reference(promise->get_reference(promise->value_reference(i)));
                int idx = root_vm->get_count();
                if      (root_vm->is_nil(idx))    results.array.emplace_back(nullptr);
                else if (root_vm->is_bool(idx))   results.array.emplace_back(root_vm->get_bool(idx));
                else if (root_vm->is_number(idx)) results.array.emplace_back(root_vm->get_double(idx));
                else if (root_vm->is_string(idx)) results.array.emplace_back(root_vm->get_string(idx));
                else                               results.array.emplace_back(nullptr);
                root_vm->pop(1);
            }

            cb(root_vm, results);
        }


        // ----------------------------------------------------------------
        // Bind
        // ----------------------------------------------------------------
        static void bind(Machine* vm) {
            Thread::register_reply_dispatcher([](int promise_id, std::shared_ptr<Promise::Instance> promise) {
                dispatch_reply(promise_id, promise);
            });

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
                h.env                = vm->get_environment_id();

                {
                    std::lock_guard lock(buffer_mutex);
                    buffer[name].handlers.emplace_back(h.exec_ref, h);
                }

                vm->push_value(true);
                return 1;
            });

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
                    vm->pop(1);
                    Promise::settle(promise, Promise::State::Resolved, vm, 0, 0);
                    push_promise(vm, promise);
                    return 1;
                }

                int args_ref = store_args_ref(vm, 2);

                std::vector<std::shared_ptr<Promise::Instance>> promises;
                fire_all(vm, std::move(snapshot), name, args_ref, FireMode::EmitCallback, &promises);
                free_ref(vm, args_ref);

                if (promises.size() == 1) {
                    push_promise(vm, promises[0]);
                } else {
                    vm->create_table();
                    for (int i = 0; i < static_cast<int>(promises.size()); ++i) {
                        push_promise(vm, promises[i]);
                        vm->set_table_field(i + 1, -2);
                    }
                }

                return 1;
            });

            // emit_remote(name, ...) -> true
            // Client: sends to server. Server: broadcasts to all peers.
            API::bind(vm, {base_name}, "emit_remote", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, ...)")
                    .require(1, &Machine::is_string);

                send_remote_emit(vm, vm->get_string(1), collect_serializable(vm, 2), 0, false);
                vm->push_value(true);
                return 1;
            });

            // emit_remote_callback(name, ...) -> Promise
            // Client: sends to server. Server: broadcasts to all peers.
            API::bind(vm, {base_name}, "emit_remote_callback", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, ...)")
                    .require(1, &Machine::is_string);

                auto promise = send_remote_emit(vm, vm->get_string(1), collect_serializable(vm, 2), 0, true);
                push_promise(vm, promise);
                return 1;
            });

            // Server-only: emit_remote_to(peer_id, name, ...) -> true
            // peer_id=0 broadcasts to all, peer_id>0 sends to that specific peer.
            #if !defined(VSDK_Client)
            API::bind(vm, {base_name}, "emit_remote_to", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(peer_id, name, ...)")
                    .require(1, &Machine::is_number)
                    .require(2, &Machine::is_string);

                send_remote_emit(vm, vm->get_string(2), collect_serializable(vm, 3), vm->get_int(1), false);
                vm->push_value(true);
                return 1;
            });

            // Server-only: emit_remote_callback_to(peer_id, name, ...) -> Promise
            // peer_id=0 broadcasts to all, peer_id>0 sends to that specific peer.
            API::bind(vm, {base_name}, "emit_remote_callback_to", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(peer_id, name, ...)")
                    .require(1, &Machine::is_number)
                    .require(2, &Machine::is_string);

                auto promise = send_remote_emit(vm, vm->get_string(2), collect_serializable(vm, 3), vm->get_int(1), true);
                push_promise(vm, promise);
                return 1;
            });
            #endif
        }

        // Called from Manager::Network::_on_packet_received.
        // Packet types:
        //   __reply_serial              — reply to a pending emit_remote_callback
        //   __event (no __serial)       — plain remote emit, fire handlers
        //   __event + __serial          — remote emit_callback, fire handlers and reply
        static void dispatch_remote(Machine* vm, const Tool::Stack& payload) {
            // Reply packet — resolve the promise on the calling side
            auto reply_ptr = payload.get("__reply_serial");
            if (reply_ptr && reply_ptr->is<double>()) {
                uint32_t serial = static_cast<uint32_t>(reply_ptr->as<double>());
                std::shared_ptr<Promise::Instance> promise;
                {
                    std::lock_guard lock(pending_remote_mutex);
                    auto it = pending_remote.find(serial);
                    if (it != pending_remote.end()) {
                        promise = it->second;
                        pending_remote.erase(it);
                    }
                }
                if (!promise) return;

                auto* root_vm = vm->get_root();
                int   base    = root_vm->get_count() + 1;
                int   count   = static_cast<int>(payload.array.size());
                for (auto& v : payload.array) root_vm->push_value(v);
                Promise::settle(promise, Promise::State::Resolved, root_vm, base, count);
                if (count > 0) root_vm->pop(count);
                return;
            }

            // Event packet — look up handlers
            auto event_name_ptr = payload.get("__event");
            if (!event_name_ptr || !event_name_ptr->is<std::string>()) return;
            std::string name = event_name_ptr->as<std::string>();

            auto     serial_ptr  = payload.get("__serial");
            bool     wants_reply = serial_ptr && serial_ptr->is<double>();
            uint32_t serial      = wants_reply ? static_cast<uint32_t>(serial_ptr->as<double>()) : 0;
            auto*    sid         = payload.get("sender_id");
            int      reply_peer  = (sid && sid->is<double>()) ? static_cast<int>(sid->as<double>()) : 0;

            std::vector<std::pair<int, Handler>> snapshot;
            {
                std::lock_guard lock(buffer_mutex);
                auto it = buffer.find(name);
                if (it != buffer.end()) snapshot = it->second.handlers;
            }

            // Build args_ref so push_args_ref passes ONE arg — the data table —
            // matching same-side emit: emit("name", myTable) → handler(myTable).
            // payload.array contains the serialized arguments sent by the caller.
            // If the caller sent a single table arg, payload.array[0] is a shared_ptr<Stack>
            // and push_value will reconstruct it as a Lua table — use it directly.
            // If the caller sent multiple scalar args, wrap them in a table.
            int stack_top = vm->get_count();

            if (payload.array.size() == 1 && payload.array[0].is<std::shared_ptr<Tool::Stack>>()) {
                // Single table arg — push it directly, then wrap in outer for push_args_ref
                vm->push_value(payload.array[0]);           // data_table at stack_top+1
            } else {
                // Multiple/scalar args — build data_table from array values
                vm->create_table();                          // data_table at stack_top+1
                int data_idx = vm->get_count();
                for (int i = 0; i < static_cast<int>(payload.array.size()); ++i) {
                    vm->push_value(payload.array[i]);
                    vm->set_table_field(i + 1, data_idx);
                }
            }

            // Wrap in outer so push_args_ref unpacks to exactly 1 arg
            int data_idx = vm->get_count();
            vm->create_table();                              // outer at stack_top+2
            int outer_idx = vm->get_count();
            vm->push(data_idx);                              // dup data_table
            vm->set_table_field(1, outer_idx);               // outer[1] = data_table

            int args_ref = vm->set_raw_reference(-1);
            vm->pop(vm->get_count() - stack_top);

            // No handlers — send empty reply if requested, then done
            if (snapshot.empty()) {
                free_ref(vm, args_ref);
                if (wants_reply) {
                    Tool::Stack reply;
                    reply.object["__reply_serial"] = Tool::StackValue(static_cast<double>(serial));
                    send_packet(reply, reply_peer);
                }
                return;
            }

            // Plain remote emit — fire and forget
            if (!wants_reply) {
                fire_all(vm, std::move(snapshot), name, args_ref, FireMode::Emit);
                free_ref(vm, args_ref);
                return;
            }

            // Remote emit_callback — fire handlers, then reply with first result
            std::vector<std::shared_ptr<Promise::Instance>> promises;
            fire_all(vm, std::move(snapshot), name, args_ref, FireMode::EmitCallback, &promises);
            free_ref(vm, args_ref);

            if (promises.empty()) return;

            register_reply_callback(promises[0], [serial, reply_peer](Machine*, const Tool::Stack& results) {
                Tool::Stack reply;
                reply.object["__reply_serial"] = Tool::StackValue(static_cast<double>(serial));
                for (auto& v : results.array) reply.array.push_back(v);
                send_packet(reply, reply_peer);
            });
        }

        // Remove all handlers registered by the given resource environment.
        static void clean(const std::string& env) {
            if (env.empty()) return;
            auto* vm = Manager::Sandbox::get_singleton()->get_vm();
            if (!vm) return;
            std::lock_guard lock(buffer_mutex);
            for (auto eit = buffer.begin(); eit != buffer.end(); ) {
                auto& vec = eit->second.handlers;
                for (auto vit = vec.begin(); vit != vec.end(); ) {
                    if (vit->second.env == env) {
                        free_ref(vm, vit->second.exec_ref);
                        vit = vec.erase(vit);
                    } else {
                        ++vit;
                    }
                }
                if (vec.empty()) eit = buffer.erase(eit);
                else ++eit;
            }
        }
    };
}