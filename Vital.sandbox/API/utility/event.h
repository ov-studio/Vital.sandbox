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

        struct HandlerConfig {
            bool is_async  = false;
            int sub_limit = 0;
        };

        struct Handler {
            int exec_ref = LUA_NOREF;
            bool async = false;
            int subscription_limit = 0;
            int subscription_count = 0;
            std::string env;
        };

        struct EventEntry {
            std::vector<std::pair<int, Handler>> handlers;
        };

        enum class FireMode { Emit, EmitCallback };

        struct EmitOptions {
            bool is_remote = false;
            int peer_id = 0;
            int args_start = 2;
        };

        inline static std::unordered_map<std::string, EventEntry> buffer;
        inline static std::mutex buffer_mutex;
        inline static std::unordered_map<uint32_t, std::shared_ptr<API::Promise::Instance>> pending_remote;
        inline static std::mutex pending_remote_mutex;
        inline static std::atomic<uint32_t> serial_counter { 0 };

        using ReplyCallback = std::function<void(Machine*, const Tool::Stack&)>;
        inline static std::unordered_map<int, ReplyCallback> reply_callbacks;
        inline static std::mutex reply_callbacks_mutex;

        static int store_ref(Machine* vm, int index) {
            vm -> push(index);
            return vm -> set_raw_reference(-1);
        }

        static void free_ref(Machine* vm, int ref) {
            if (ref != LUA_NOREF) vm -> del_raw_reference(ref);
        }
    
        static HandlerConfig read_config(Machine* vm, int index) {
            HandlerConfig cfg;
            if (vm -> get_count() < index || !vm -> is_table(index)) return cfg;
            vm -> get_table_field("async", index); cfg.is_async  = vm -> get_bool(-1); vm -> pop(1);
            vm -> get_table_field("subscription_limit", index);
            if (vm -> is_number(-1)) cfg.sub_limit = std::max(1, vm -> get_int(-1));
            vm -> pop(1);
            return cfg;
        }

        static EmitOptions read_emit_options(Machine* vm) {
            EmitOptions opts;
            if (vm -> get_count() >= 2 && vm -> is_table(2)) {
                vm -> get_table_field("remote", 2);
                if (!vm -> is_nil(-1)) opts.is_remote = vm -> get_bool(-1);
                vm -> pop(1);
                #if !defined(VSDK_Client)
                vm -> get_table_field("peer", 2);
                if (vm -> is_number(-1)) opts.peer_id = vm -> get_int(-1);
                vm -> pop(1);
                #endif
                opts.args_start = 3;
            }
            return opts;
        }

        static int store_args_ref(Machine* vm, int from_index) {
            vm -> create_table();
            int top   = vm -> get_count() - 1;
            int count = top - from_index;
            for (int i = 0; i <= count; ++i) {
                vm -> push(from_index + i);
                vm -> set_table_field(i + 1, -2);
            }
            return vm -> set_raw_reference(-1);
        }

        static int push_args_ref(Machine* vm, int args_ref) {
            vm -> get_raw_reference(args_ref);
            int table_idx = vm -> get_count();
            int n = vm -> get_length(table_idx);
            for (int i = 1; i <= n; ++i)
                vm -> get_table_field(i, table_idx);
            vm -> rotate(table_idx, -1);
            vm -> pop(1);
            return n;
        }

        static void push_promise(Machine* vm, std::shared_ptr<API::Promise::Instance> promise) {
            vm -> get_raw_reference(promise -> get_reference(promise -> self_reference()));
        }

        static Tool::StackValue collect_stack_value(Machine* vm, int index, std::unordered_set<const void*>& visited, int depth = 0) {
            auto* L = vm -> get_state();
            switch (lua_type(L, index)) {
                case LUA_TNIL: return Tool::StackValue(nullptr);
                case LUA_TBOOLEAN: return Tool::StackValue((bool)lua_toboolean(L, index));
                case LUA_TNUMBER: return Tool::StackValue((double)lua_tonumber(L, index));
                case LUA_TSTRING: return Tool::StackValue(std::string(lua_tostring(L, index)));
                case LUA_TTABLE: return Tool::StackValue(collect_table(vm, index, visited, depth));
                default: return Tool::StackValue(nullptr);
            }
        }

        static std::shared_ptr<Tool::Stack> collect_table(Machine* vm, int index, std::unordered_set<const void*>& visited, int depth = 0) {
            auto stack = std::make_shared<Tool::Stack>();
            if (depth > 32) return stack;

            auto* L = vm -> get_state();
            const void* ptr = lua_topointer(L, index);
            if (!ptr || visited.count(ptr)) return stack;
            visited.insert(ptr);

            lua_pushnil(L);
            while (lua_next(L, index) != 0) {
                int key_type = lua_type(L, -2);
                int val_type = lua_type(L, -1);
                if (key_type == LUA_TNUMBER) {
                    int idx = (int)lua_tonumber(L, -2);
                    if (idx >= 1) {
                        Tool::StackValue val = collect_stack_value(vm, lua_gettop(L), visited, depth + 1);
                        if (static_cast<int>(stack -> array.size()) < idx)
                            stack -> array.resize(idx, Tool::StackValue(nullptr));
                        stack -> array[idx - 1] = val;
                    }
                }
                else if (key_type == LUA_TSTRING) {
                    if (val_type == LUA_TNIL || 
                        val_type == LUA_TBOOLEAN || 
                        val_type == LUA_TNUMBER || 
                        val_type == LUA_TSTRING || 
                        val_type == LUA_TTABLE
                    ) stack -> object[lua_tostring(L, -2)] = collect_stack_value(vm, lua_gettop(L), visited, depth + 1);
                }
                lua_pop(L, 1);
            }
            visited.erase(ptr);
            return stack;
        }

        static Tool::Stack collect_serializable(Machine* vm, int from_index) {
            Tool::Stack payload;
            std::unordered_set<const void*> visited;
            int top = vm -> get_count();
            for (int i = from_index; i <= top; ++i) payload.array.emplace_back(collect_stack_value(vm, i, visited));
            return payload;
        }

        static void send_packet(const Tool::Stack& payload, int peer_id = 0) {
            #if defined(VSDK_Client)
                Manager::Network::get_singleton() -> send_to_server(payload);
            #else
                if (peer_id > 0) Manager::Network::get_singleton() -> send(payload, peer_id);
                else Manager::Network::get_singleton() -> broadcast(payload);
            #endif
        }

        static std::shared_ptr<API::Promise::Instance> send_remote_emit(Machine* vm, const std::string& name, Tool::Stack payload, int peer_id, bool wants_callback) {
            payload.object["__event"] = Tool::StackValue(name);
            std::shared_ptr<API::Promise::Instance> promise;
            if (wants_callback) {
                uint32_t serial = ++serial_counter;
                payload.object["__serial"] = Tool::StackValue(static_cast<double>(serial));
                promise = API::Promise::make(vm);
                vm -> pop(1);
                {
                    std::lock_guard lock(pending_remote_mutex);
                    pending_remote[serial] = promise;
                }
            }
            send_packet(payload, peer_id);
            return promise;
        }

        static void bump_subscription(const std::string& name, int ref, std::vector<int>& exhausted) {
            std::lock_guard lock(buffer_mutex);
            auto it = buffer.find(name);
            if (it == buffer.end()) return;
            for (auto& [vref, vh] : it -> second.handlers) {
                if (vref != ref) continue;
                vh.subscription_count++;
                if (vh.subscription_count >= vh.subscription_limit) exhausted.push_back(ref);
                return;
            }
        }

        static void sweep_exhausted(Machine* vm, const std::string& name, const std::vector<int>& dead_refs) {
            if (dead_refs.empty()) return;
            std::lock_guard lock(buffer_mutex);
            auto it = buffer.find(name);
            if (it == buffer.end()) return;
            for (auto vit = it -> second.handlers.begin(); vit != it -> second.handlers.end(); ) {
                if (std::find(dead_refs.begin(), dead_refs.end(), vit -> first) != dead_refs.end()) {
                    free_ref(vm, vit -> second.exec_ref);
                    vit = it -> second.handlers.erase(vit);
                }
                else ++vit;
            }
        }

        static void spawn_thread(Machine* vm, int exec_ref, int args_ref, std::shared_ptr<API::Promise::Instance> promise = nullptr) {
            auto* root_vm = vm -> get_root();
            vm -> get_raw_reference(exec_ref);
            auto instance = API::Thread::make(vm);
            vm -> get_raw_reference(exec_ref);
            int n_args = push_args_ref(vm, args_ref);
            vm -> move(instance -> thread_vm, 1 + n_args);

            auto weak_promise = std::weak_ptr<API::Promise::Instance>(promise);
            instance -> thread_vm -> set_finish_hook([weak_promise, root_vm, instance](Machine* thread_vm, int nresults) {
                instance -> thread_state = nullptr;
                auto p = weak_promise.lock();
                if (!p) return;
                int base = root_vm -> get_count() + 1;
                if (nresults > 0) lua_xmove(thread_vm -> get_state(), root_vm -> get_state(), nresults);
                API::Promise::settle(p, API::Promise::State::Resolved, root_vm, base, nresults);
                if (nresults > 0) root_vm -> pop(nresults);
            });
            API::Thread::safe_resume(instance, n_args);
        }

        static std::shared_ptr<API::Promise::Instance> fire_one(Machine* vm, const Handler& h, int args_ref, FireMode mode) {
            std::shared_ptr<API::Promise::Instance> promise;
            if (mode == FireMode::EmitCallback) {
                promise = API::Promise::make(vm);
                vm -> pop(1);
            }

            if (h.async) {
                vm -> get_raw_reference(args_ref);
                vm -> create_table();
                int orig = vm -> get_count() - 1, copy = vm -> get_count();
                int n = vm -> get_length(orig);
                for (int i = 1; i <= n; ++i) {
                    vm -> get_table_field(i, orig);
                    vm -> set_table_field(i, copy);
                }
                vm -> rotate(-2, -1);
                vm -> pop(1);
                int args_ref_copy = vm -> set_raw_reference(-1);
                spawn_thread(vm, h.exec_ref, args_ref_copy, promise);
                free_ref(vm, args_ref_copy);
            }
            else {
                vm -> get_raw_reference(h.exec_ref);
                int n_args = push_args_ref(vm, args_ref);
                if (mode == FireMode::EmitCallback) {
                    int base = vm -> get_count() - n_args - 1;
                    if (vm -> pcall(n_args, LUA_MULTRET)) {
                        int result_count = vm -> get_count() - base;
                        API::Promise::settle(promise, API::Promise::State::Resolved, vm, base + 1, result_count);
                        vm -> pop(result_count);
                    }
                    else API::Promise::settle(promise, API::Promise::State::Rejected, vm, 0, 0);
                }
                else vm -> pcall(n_args, 0);
            }
            return promise;
        }

        static void fire_all(Machine* vm, std::vector<std::pair<int, Handler>> snapshot, const std::string& name, int args_ref, FireMode mode, std::vector<std::shared_ptr<API::Promise::Instance>>* promises = nullptr) {
            std::vector<int> exhausted;
            for (auto& [ref, h] : snapshot) {
                auto promise = fire_one(vm, h, args_ref, mode);
                if (promise && promises) promises -> push_back(promise);
                if (h.subscription_limit > 0) bump_subscription(name, ref, exhausted);
            }
            sweep_exhausted(vm, name, exhausted);
        }

        static void emit_internal(Machine* vm, const std::string& name, const Tool::Stack& stack) {
            std::vector<std::pair<int, Handler>> snapshot;
            {
                std::lock_guard lock(buffer_mutex);
                auto it = buffer.find(name);
                if (it != buffer.end()) snapshot = it -> second.handlers;
            }
            if (snapshot.empty()) return;

            int stack_top = vm -> get_count();
            vm -> create_table();
            int outer_idx = vm -> get_count();
            for (int i = 0; i < static_cast<int>(stack.array.size()); ++i) {
                vm -> push_value(stack.array[i]);
                vm -> set_table_field(i + 1, outer_idx);
            }
            int args_ref = vm -> set_raw_reference(-1);
            vm -> pop(vm -> get_count() - stack_top);
            fire_all(vm, std::move(snapshot), name, args_ref, FireMode::Emit);
            free_ref(vm, args_ref);
        }

        static std::shared_ptr<API::Promise::Instance> aggregate_promises(Machine* vm, std::vector<std::shared_ptr<API::Promise::Instance>>& per_handler) {
            if (per_handler.empty()) {
                auto p = API::Promise::make(vm);
                vm -> pop(1);
                API::Promise::settle(p, API::Promise::State::Resolved, vm, 0, 0);
                return p;
            }
            if (per_handler.size() == 1) return per_handler[0];

            // TODO: Move outside to top level?
            struct AggState {
                std::mutex mtx;
                int total;
                std::atomic<int> done { 0 };
                std::vector<std::vector<Tool::StackValue>> results;
                std::weak_ptr<API::Promise::Instance> agg_promise;
                Machine* vm;
            };

            auto agg = API::Promise::make(vm);
            vm -> pop(1);
            auto state = std::make_shared<AggState>();
            state -> total = static_cast<int>(per_handler.size());
            state -> results.resize(state -> total);
            state -> agg_promise = agg;
            state -> vm = vm -> get_root();

            for (int i = 0; i < state -> total; ++i) {
                auto& p = per_handler[i];
                int slot = i;
                if (p -> state != API::Promise::State::Pending) {
                    auto* root_vm = vm -> get_root();
                    for (int j = 1; j <= p -> values; ++j) {
                        root_vm -> get_raw_reference(p -> get_reference(p -> value_reference(j)));
                        std::unordered_set<const void*> vis;
                        state -> results[slot].push_back(collect_stack_value(root_vm, root_vm -> get_count(), vis));
                        root_vm -> pop(1);
                    }
                    if (++state -> done == state -> total) settle_aggregate(state);
                    continue;
                }
                {
                    std::lock_guard lock(reply_callbacks_mutex);
                    reply_callbacks[p -> id] = [state, slot](Machine* root_vm, const Tool::Stack& results) {
                        state -> results[slot] = results.array;
                        if (++state -> done == state -> total) settle_aggregate(state);
                    };
                }
                p -> waiting.push_back(-1);
            }
            return agg;
        }

        struct AggState;
        template<typename T>
        static void settle_aggregate(std::shared_ptr<T> state) {
            Machine::enqueue([state]() {
                auto agg = state -> agg_promise.lock();
                if (!agg) return;

                auto* root_vm = state -> vm;
                root_vm -> create_table();
                int tbl = root_vm -> get_count();
                for (int i = 0; i < static_cast<int>(state -> results.size()); ++i) {
                    auto& row = state -> results[i];
                    root_vm -> create_table();
                    int sub = root_vm -> get_count();
                    for (int j = 0; j < static_cast<int>(row.size()); ++j) {
                        root_vm -> push_value(row[j]);
                        root_vm -> set_table_field(j + 1, sub);
                    }
                    root_vm -> set_table_field(i + 1, tbl);
                }
                int base = tbl;
                API::Promise::settle(agg, API::Promise::State::Resolved, root_vm, base, 1);
                root_vm -> pop(1);
            });
        }

        static void register_reply_callback(std::shared_ptr<API::Promise::Instance> promise, ReplyCallback cb) {
            {
                std::lock_guard lock(reply_callbacks_mutex);
                reply_callbacks[promise -> id] = std::move(cb);
            }
            promise -> waiting.push_back(-1);
        }

        static void dispatch_reply(int promise_id, std::shared_ptr<API::Promise::Instance> promise) {
            ReplyCallback cb;
            {
                std::lock_guard lock(reply_callbacks_mutex);
                auto it = reply_callbacks.find(promise_id);
                if (it == reply_callbacks.end()) return;
                cb = std::move(it -> second);
                reply_callbacks.erase(it);
            }
            auto* root_vm = Manager::Sandbox::get_singleton() -> get_vm();
            if (!root_vm || !cb) return;

            Tool::Stack results;
            std::unordered_set<const void*> visited;
            for (int i = 1; i <= promise -> values; ++i) {
                root_vm -> get_raw_reference(promise -> get_reference(promise -> value_reference(i)));
                results.array.emplace_back(collect_stack_value(root_vm, root_vm -> get_count(), visited));
                root_vm -> pop(1);
            }
            cb(root_vm, results);
        }

        static void bind(Machine* vm) {
            API::Thread::register_reply_dispatcher([](int promise_id, std::shared_ptr<API::Promise::Instance> promise) {
                dispatch_reply(promise_id, promise);
            });

            Manager::Sandbox::register_signal_dispatcher([](Machine* vm, const std::string& name, const Tool::Stack& stack) {
                emit_internal(vm, name, stack);
            });

            API::bind(vm, {base_name}, "on", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, exec, config = nil)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_function);

                std::string name = vm -> get_string(1);
                HandlerConfig cfg = read_config(vm, 3);
                Handler h;
                h.exec_ref = store_ref(vm, 2);
                h.async = cfg.is_async;
                h.subscription_limit = cfg.sub_limit;
                h.subscription_count = 0;
                h.env = vm -> get_environment_id();
                {
                    std::lock_guard lock(buffer_mutex);
                    buffer[name].handlers.emplace_back(h.exec_ref, h);
                }
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "off", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, exec)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_function);

                std::string name = vm -> get_string(1);
                bool removed = false;
                vm -> push(2);
                int lookup_ref = vm -> set_raw_reference(-1);
                {
                    std::lock_guard lock(buffer_mutex);
                    auto it = buffer.find(name);
                    if (it != buffer.end()) {
                        auto& vec = it -> second.handlers;
                        for (auto vit = vec.begin(); vit != vec.end(); ++vit) {
                            vm -> get_raw_reference(lookup_ref);
                            vm -> get_raw_reference(vit -> second.exec_ref);
                            bool eq = lua_rawequal(vm -> get_state(), -1, -2);
                            vm -> pop(2);
                            if (!eq) continue;
                            free_ref(vm, vit -> second.exec_ref);
                            vec.erase(vit);
                            removed = true;
                            break;
                        }
                    }
                }
                free_ref(vm, lookup_ref);
                vm -> push_value(removed);
                return 1;
            });

            API::bind(vm, {base_name}, "emit", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, options = nil, ...)")
                    .require(1, &Machine::is_string);

                std::string name = vm -> get_string(1);
                auto opts = read_emit_options(vm);
                if (opts.is_remote) send_remote_emit(vm, name, collect_serializable(vm, opts.args_start), opts.peer_id, false);
                else {
                    std::vector<std::pair<int, Handler>> snapshot;
                    {
                        std::lock_guard lock(buffer_mutex);
                        auto it = buffer.find(name);
                        if (it != buffer.end()) snapshot = it -> second.handlers;
                    }
                    if (!snapshot.empty()) {
                        int args_ref = store_args_ref(vm, opts.args_start);
                        fire_all(vm, std::move(snapshot), name, args_ref, FireMode::Emit);
                        free_ref(vm, args_ref);
                    }
                }
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, {base_name}, "emit_callback", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, options = nil, ...)")
                    .require(1, &Machine::is_string);

                std::string name = vm -> get_string(1);
                auto opts = read_emit_options(vm);
                if (opts.is_remote) {
                    auto promise = send_remote_emit(vm, name, collect_serializable(vm, opts.args_start), opts.peer_id, true);
                    push_promise(vm, promise);
                    return 1;
                }

                std::vector<std::pair<int, Handler>> snapshot;
                {
                    std::lock_guard lock(buffer_mutex);
                    auto it = buffer.find(name);
                    if (it != buffer.end()) snapshot = it -> second.handlers;
                }
                if (snapshot.empty()) {
                    auto promise = API::Promise::make(vm);
                    vm -> pop(1);
                    API::Promise::settle(promise, API::Promise::State::Resolved, vm, 0, 0);
                    push_promise(vm, promise);
                    return 1;
                }

                int args_ref = store_args_ref(vm, opts.args_start);
                std::vector<std::shared_ptr<API::Promise::Instance>> promises;
                fire_all(vm, std::move(snapshot), name, args_ref, FireMode::EmitCallback, &promises);
                free_ref(vm, args_ref);
                auto agg = aggregate_promises(vm, promises);
                push_promise(vm, agg);
                return 1;
            });
        }

        static void dispatch_remote(Machine* vm, const Tool::Stack& payload) {
            auto reply_ptr = payload.get("__reply_serial");
            if (reply_ptr && reply_ptr -> is<double>()) {
                uint32_t serial = static_cast<uint32_t>(reply_ptr -> as<double>());
                std::shared_ptr<API::Promise::Instance> promise;
                {
                    std::lock_guard lock(pending_remote_mutex);
                    auto it = pending_remote.find(serial);
                    if (it != pending_remote.end()) {
                        promise = it -> second;
                        pending_remote.erase(it);
                    }
                }
                if (!promise) return;

                auto* root_vm = vm -> get_root();
                int base = root_vm -> get_count() + 1;
                int count = static_cast<int>(payload.array.size());
                for (auto& v : payload.array) root_vm -> push_value(v);
                API::Promise::settle(promise, API::Promise::State::Resolved, root_vm, base, count);
                if (count > 0) root_vm -> pop(count);
                return;
            }

            auto event_name_ptr = payload.get("__event");
            if (!event_name_ptr || !event_name_ptr -> is<std::string>()) return;
            std::string name = event_name_ptr -> as<std::string>();

            auto serial_ptr = payload.get("__serial");
            bool wants_reply = serial_ptr && serial_ptr -> is<double>();
            uint32_t serial = wants_reply ? static_cast<uint32_t>(serial_ptr -> as<double>()) : 0;
            auto* sid = payload.get("sender_id");
            int reply_peer = (sid && sid -> is<double>()) ? static_cast<int>(sid -> as<double>()) : 0;

            std::vector<std::pair<int, Handler>> snapshot;
            {
                std::lock_guard lock(buffer_mutex);
                auto it = buffer.find(name);
                if (it != buffer.end()) snapshot = it -> second.handlers;
            }

            int stack_top = vm -> get_count();
            vm -> create_table();
            int outer_idx = vm -> get_count();
            for (int i = 0; i < static_cast<int>(payload.array.size()); ++i) {
                vm -> push_value(payload.array[i]);
                vm -> set_table_field(i + 1, outer_idx);
            }
            int args_ref = vm -> set_raw_reference(-1);
            vm -> pop(vm -> get_count() - stack_top);

            if (snapshot.empty()) {
                free_ref(vm, args_ref);
                if (wants_reply) {
                    Tool::Stack reply;
                    reply.object["__reply_serial"] = Tool::StackValue(static_cast<double>(serial));
                    send_packet(reply, reply_peer);
                }
                return;
            }

            if (!wants_reply) {
                fire_all(vm, std::move(snapshot), name, args_ref, FireMode::Emit);
                free_ref(vm, args_ref);
                return;
            }

            std::vector<std::shared_ptr<API::Promise::Instance>> promises;
            fire_all(vm, std::move(snapshot), name, args_ref, FireMode::EmitCallback, &promises);
            free_ref(vm, args_ref);
            if (promises.empty()) return;

            auto agg = aggregate_promises(vm, promises);
            register_reply_callback(agg, [serial, reply_peer](Machine* root_vm, const Tool::Stack& results) {
                Tool::Stack reply;
                reply.object["__reply_serial"] = Tool::StackValue(static_cast<double>(serial));
                for (auto& v : results.array) reply.array.push_back(v);
                send_packet(reply, reply_peer);
            });
        }

        static void clean(const std::string& env) {
            if (env.empty()) return;
            auto vm = Manager::Sandbox::get_singleton() -> get_vm();
            if (!vm) return;

            std::lock_guard lock(buffer_mutex);
            for (auto eit = buffer.begin(); eit != buffer.end(); ) {
                auto& vec = eit -> second.handlers;
                for (auto vit = vec.begin(); vit != vec.end(); ) {
                    if (vit -> second.env == env) {
                        free_ref(vm, vit -> second.exec_ref);
                        vit = vec.erase(vit);
                    }
                    else ++vit;
                }
                if (vec.empty()) eit = buffer.erase(eit);
                else ++eit;
            }
        }
    };
}