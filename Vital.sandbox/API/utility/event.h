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
    struct Event : vm_module {
        inline static const std::string base_nspace = "util";
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

        struct AggState {
            int total;
            std::atomic<int> done { 0 };
            std::vector<std::vector<Tool::StackValue>> results;
            std::weak_ptr<API::Promise::Instance> agg_promise;
            Machine* vm;
        };

        inline static std::unordered_map<std::string, EventEntry> buffer;
        inline static std::mutex buffer_mutex;
        inline static std::unordered_map<uint32_t, std::shared_ptr<API::Promise::Instance>> pending_remote;
        inline static std::mutex pending_remote_mutex;
        inline static std::atomic<uint32_t> serial_counter { 0 };

        using ReplyCallback = std::function<void(Machine*, const Tool::Stack&)>;
        inline static std::unordered_map<int, ReplyCallback> reply_callbacks;
        inline static std::mutex reply_callbacks_mutex;

        static HandlerConfig read_config(Machine* vm, int index) {
            HandlerConfig cfg;
            if (vm -> get_count() < index || !vm -> is_table(index)) return cfg;
            vm -> get_table_field("async", index); cfg.is_async = vm -> get_bool(-1); vm -> pop(1);
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

        static void push_promise(Machine* vm, std::shared_ptr<API::Promise::Instance> promise) {
            vm -> get_raw_reference(promise -> get_reference(promise -> self_reference()));
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
                    vm -> del_raw_reference(vit -> second.exec_ref);
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
            int n_args = vm -> push_args(args_ref);
            vm -> move(instance -> thread_vm, 1 + n_args);

            auto weak_promise = std::weak_ptr<API::Promise::Instance>(promise);
            instance -> thread_vm -> set_finish_hook([weak_promise, root_vm, instance](Machine* thread_vm, int nresults) {
                instance -> thread_state = nullptr;
                auto promise = weak_promise.lock();
                if (!promise) return;
                int base = root_vm -> get_count() + 1;
                if (nresults > 0) thread_vm -> move(root_vm, nresults);
                API::Promise::settle(promise, API::Promise::State::Resolved, root_vm, base, nresults);
                if (nresults > 0) root_vm -> pop(nresults);
            });
            API::Thread::safe_resume(instance, n_args);
        }

        static std::shared_ptr<API::Promise::Instance> fire_one(Machine* vm, const Handler& h, int args_ref, FireMode mode) {
            std::shared_ptr<API::Promise::Instance> promise;
            if (mode == FireMode::EmitCallback) promise = API::Promise::make(vm);
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
                vm -> del_raw_reference(args_ref_copy);
            }
            else {
                vm -> get_raw_reference(h.exec_ref);
                int n_args = vm -> push_args(args_ref);
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

        static void settle_aggregate(std::shared_ptr<AggState> state) {
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

        static std::shared_ptr<API::Promise::Instance> aggregate_promises(Machine* vm, std::vector<std::shared_ptr<API::Promise::Instance>>& per_handler) {
            if (per_handler.empty()) {
                auto promise = API::Promise::make(vm);
                API::Promise::settle(promise, API::Promise::State::Resolved, vm, 0, 0);
                return promise;
            }
            if (per_handler.size() == 1) return per_handler[0];

            auto agg = API::Promise::make(vm);
            auto agg_state = std::make_shared<AggState>();
            agg_state -> total = static_cast<int>(per_handler.size());
            agg_state -> results.resize(agg_state -> total);
            agg_state -> agg_promise = agg;
            agg_state -> vm = vm -> get_root();

            for (int i = 0; i < agg_state -> total; ++i) {
                auto& promise = per_handler[i];
                int slot = i;
                if (promise -> state != API::Promise::State::Pending) {
                    auto* root_vm = vm -> get_root();
                    for (int j = 1; j <= promise -> values; ++j) {
                        root_vm -> get_raw_reference(promise -> get_reference(promise -> value_reference(j)));
                        std::unordered_set<const void*> vis;
                        agg_state -> results[slot].push_back(root_vm -> collect_value(root_vm -> get_count(), vis));
                        root_vm -> pop(1);
                    }
                    if (++agg_state -> done == agg_state -> total) settle_aggregate(agg_state);
                    continue;
                }
                {
                    std::lock_guard lock(reply_callbacks_mutex);
                    reply_callbacks[promise -> id] = [agg_state, slot](Machine* root_vm, const Tool::Stack& results) {
                        agg_state -> results[slot] = results.array;
                        if (++agg_state -> done == agg_state -> total) settle_aggregate(agg_state);
                    };
                }
                promise -> waiting.push_back(-1);
            }
            return agg;
        }

        static void register_reply_callback(std::shared_ptr<API::Promise::Instance> promise, ReplyCallback cb) {
            {
                std::lock_guard lock(reply_callbacks_mutex);
                reply_callbacks[promise -> id] = std::move(cb);
            }
            promise -> waiting.push_back(-1);
        }

        static void init(Machine* vm) {
            static bool initialized = false;
            if (initialized) return;
            initialized = true;
            
            Tool::Event::bind("promise:reply", [](Tool::Stack args) {
                if (args.array.size() < 1) return;
                if (!args.array[0].is_ptr<API::Promise::Instance>()) return;

                auto promise = args.array[0].as_ptr<API::Promise::Instance>();
                ReplyCallback cb;
                {
                    std::lock_guard lock(reply_callbacks_mutex);
                    auto it = reply_callbacks.find(promise -> id);
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
                    results.array.emplace_back(root_vm -> collect_value(root_vm -> get_count(), visited));
                    root_vm -> pop(1);
                }
                cb(root_vm, results);
            });

            Tool::Event::bind("sandbox:signal", [](Tool::Stack args) {
                if (args.array.size() < 2) return;
                if (!args.array[0].is<std::string>() || !args.array[1].is<std::shared_ptr<Tool::Stack>>()) return;
                auto stack = args.array[1].as<std::shared_ptr<Tool::Stack>>();
                if (!stack) return;
                auto* vm = Manager::Sandbox::get_singleton() -> get_vm();
                if (!vm) return;
            
                std::string name = args.array[0].as<std::string>();
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
                for (int i = 0; i < static_cast<int>(stack -> array.size()); ++i) {
                    vm -> push_value(stack -> array[i]);
                    vm -> set_table_field(i + 1, outer_idx);
                }
                int args_ref = vm -> set_raw_reference(-1);
                vm -> pop(vm -> get_count() - stack_top);
                fire_all(vm, std::move(snapshot), name, args_ref, FireMode::Emit);
                vm -> del_raw_reference(args_ref);
            });
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "on", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, exec, config = nil)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_function);

                std::string name = vm -> get_string(1);
                HandlerConfig cfg = read_config(vm, 3);
                Handler h;
                h.exec_ref = vm -> set_raw_reference(2);
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

            API::bind(vm, base_scope, "off", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, exec)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_function);

                std::string name = vm -> get_string(1);
                bool removed = false;
                int lookup_ref = vm -> set_raw_reference(2);
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
                            vm -> del_raw_reference(vit -> second.exec_ref);
                            vec.erase(vit);
                            removed = true;
                            break;
                        }
                    }
                }
                vm -> del_raw_reference(lookup_ref);
                vm -> push_value(removed);
                return 1;
            });

            API::bind(vm, base_scope, "emit", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, options = nil, ...)")
                    .require(1, &Machine::is_string);

                std::string name = vm -> get_string(1);
                auto opts = read_emit_options(vm);
                if (opts.is_remote) send_remote_emit(vm, name, vm -> collect_args(opts.args_start), opts.peer_id, false);
                else {
                    std::vector<std::pair<int, Handler>> snapshot;
                    {
                        std::lock_guard lock(buffer_mutex);
                        auto it = buffer.find(name);
                        if (it != buffer.end()) snapshot = it -> second.handlers;
                    }
                    if (!snapshot.empty()) {
                        int args_ref = vm -> store_args(opts.args_start);
                        fire_all(vm, std::move(snapshot), name, args_ref, FireMode::Emit);
                        vm -> del_raw_reference(args_ref);
                    }
                }
                vm -> push_value(true);
                return 1;
            });

            API::bind(vm, base_scope, "emit_callback", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, options = nil, ...)")
                    .require(1, &Machine::is_string);

                std::string name = vm -> get_string(1);
                auto opts = read_emit_options(vm);
                if (opts.is_remote) {
                    auto promise = send_remote_emit(vm, name, vm -> collect_args(opts.args_start), opts.peer_id, true);
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
                    API::Promise::settle(promise, API::Promise::State::Resolved, vm, 0, 0);
                    push_promise(vm, promise);
                    return 1;
                }

                int args_ref = vm -> store_args(opts.args_start);
                std::vector<std::shared_ptr<API::Promise::Instance>> promises;
                fire_all(vm, std::move(snapshot), name, args_ref, FireMode::EmitCallback, &promises);
                vm -> del_raw_reference(args_ref);
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
                vm -> del_raw_reference(args_ref);
                if (wants_reply) {
                    Tool::Stack reply;
                    reply.object["__reply_serial"] = Tool::StackValue(static_cast<double>(serial));
                    send_packet(reply, reply_peer);
                }
                return;
            }

            if (!wants_reply) {
                fire_all(vm, std::move(snapshot), name, args_ref, FireMode::Emit);
                vm -> del_raw_reference(args_ref);
                return;
            }

            std::vector<std::shared_ptr<API::Promise::Instance>> promises;
            fire_all(vm, std::move(snapshot), name, args_ref, FireMode::EmitCallback, &promises);
            vm -> del_raw_reference(args_ref);
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
                        vm -> del_raw_reference(vit -> second.exec_ref);
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