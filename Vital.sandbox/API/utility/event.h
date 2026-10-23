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
#include <Vital.sandbox/Manager/public/resource.h>
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/API/utility/promise.h>
#include <Vital.sandbox/API/utility/thread.h>


////////////////////////
// Vital: API: Event //
////////////////////////

// TODO: Improve
namespace Vital::Sandbox::API {
    struct Event : vm_module {
        inline static const std::vector<std::string> base_scope = {"util", "event"};
        inline static constexpr bool pre_cleanup = false;

        struct Handler {
            std::string env;
            int exec_ref = LUA_NOREF;
            bool async = false;
            int subscription_limit = 0;
            int subscription_count = 0;
            int priority = 0;
        };

        struct EventEntry {
            std::vector<std::pair<int, Handler>> handlers;
        };

        enum class FireMode {
            Emit,
            EmitCallback
        };

        struct EmitOptions {
            bool remote = false;
            int peer_id = 0;
            int args_start = 2;
            bool collect_all = false;
        };

        struct AggState {
            int total;
            std::atomic<int> done { 0 };
            std::vector<std::vector<Tool::StackValue>> results;
            std::weak_ptr<API::Promise::Instance> agg_promise;
            Machine* vm;
        };

        struct PendingRemote {
            std::shared_ptr<API::Promise::Instance> promise;
            std::chrono::steady_clock::time_point created;
            int peer = 0;
        };

        using ReplyCallback = std::function<void(Machine*, const Tool::Stack&)>;
        inline static constexpr int remote_timeout_ms = 30000;
        inline static constexpr int remote_sweep_interval_ms = 5000;
        inline static int dispatch_depth = 0;
        inline static std::unordered_map<std::string, EventEntry> buffer;
        inline static std::mutex buffer_mutex;
        inline static std::unordered_map<uint32_t, PendingRemote> pending_remote;
        inline static std::mutex pending_remote_mutex;
        inline static std::atomic<uint32_t> serial_counter { 0 };
        inline static std::unordered_map<int, ReplyCallback> reply_callbacks;
        inline static std::mutex reply_callbacks_mutex;
        inline static std::unordered_map<std::string, std::vector<Tool::Stack>> pending_resource_remote;
        inline static std::mutex pending_resource_remote_mutex;

        static void read_config(Machine* vm, int idx, Handler& handler) {
            if ((vm -> get_count() < idx) || !vm -> is_table(idx)) return;
            vm -> table_get_value("async", idx); handler.async = vm -> get_bool(-1); vm -> pop(1);
            vm -> table_get_value("subscription_limit", idx);
            if (vm -> is_number(-1)) handler.subscription_limit = std::max(1, vm -> get_int(-1));
            vm -> pop(1);
            vm -> table_get_value("priority", idx);
            if (vm -> is_number(-1)) handler.priority = vm -> get_int(-1);
            vm -> pop(1);
        }

        static void insert_ordered(EventEntry& entry, int ref, Handler handler) {
            auto& vec = entry.handlers;
            auto pos = vec.begin();
            while (pos != vec.end() && pos -> second.priority >= handler.priority) ++pos;
            vec.emplace(pos, ref, std::move(handler));
        }

        static EmitOptions read_emit_options(Machine* vm) {
            EmitOptions opts;
            if (vm -> get_count() >= 2 && vm -> is_table(2)) {
                vm -> table_get_value("remote", 2);
                if (!vm -> is_nil(-1)) opts.remote = vm -> get_bool(-1);
                vm -> pop(1);
                vm -> table_get_value("collect_all", 2);
                if (!vm -> is_nil(-1)) opts.collect_all = vm -> get_bool(-1);
                vm -> pop(1);
                #if !defined(VSDK_Client)
                vm -> table_get_value("peer", 2);
                if (vm -> is_number(-1)) opts.peer_id = vm -> get_int(-1);
                vm -> pop(1);
                #endif
                opts.args_start = 3;
            }
            return opts;
        }

        #if !defined(VSDK_Client)
        static void verify_peer(const std::string& caller, int peer_id, bool required) {
            if (peer_id <= 0) {
                if (required) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("'options.peer' is required for remote {} on server", caller));
                return;
            }
            Manager::Network::get_singleton() -> assert_peer(peer_id);
        }
        #endif

        static void push_promise(Machine* vm, std::shared_ptr<API::Promise::Instance> promise) {
            vm -> get_raw_reference(promise -> get_reference(promise -> self_reference()));
        }

        static int store_stack_args(Machine* vm, const std::vector<Tool::StackValue>& array) {
            int stack_top = vm -> get_count();
            vm -> create_table();
            int outer_idx = vm -> get_count();
            for (int i = 0; i < static_cast<int>(array.size()); ++i) {
                vm -> push_value(array[i]);
                vm -> set_table_field(i + 1, outer_idx);
            }
            int args_ref = vm -> set_raw_reference(-1);
            vm -> pop(vm -> get_count() - stack_top);
            return args_ref;
        }
    
        static void send_packet(const Tool::Stack& payload, int peer_id = 0) {
            #if defined(VSDK_Client)
                Manager::Network::get_singleton() -> send_to_server(payload);
            #else
                if (peer_id > 0) Manager::Network::get_singleton() -> send(payload, peer_id);
                else Manager::Network::get_singleton() -> broadcast(payload);
            #endif
        }

        // Authenticated transport sender - written by Network::_on_packet_received after the payload is decoded, so it can't be spoofed by the remote side
        static bool get_sender(const Tool::Stack& payload, int& sender) {
            auto ptr = payload.get("sender");
            if (!ptr || !ptr -> is<int32_t>()) return false;
            sender = ptr -> as<int32_t>();
            return true;
        }

        static bool to_serial(double value, uint32_t& serial) {
            if (!(value >= 1.0 && value <= 4294967295.0)) return false;
            serial = static_cast<uint32_t>(value);
            return true;
        }

        static void send_reply(uint32_t serial, int reply_peer, const std::vector<Tool::StackValue>* results = nullptr, bool failed = false) {
            #if !defined(VSDK_Client)
            if (reply_peer <= 0) return;
            #endif
            Tool::Stack reply;
            reply.object["__reply_serial"] = Tool::StackValue(static_cast<double>(serial));
            if (failed) reply.object["__reply_error"] = Tool::StackValue(true);
            if (results) for (auto& v : *results) reply.array.push_back(v);
            send_packet(reply, reply_peer);
        }

        static std::shared_ptr<API::Promise::Instance> send_remote_emit(Machine* vm, const std::string& name, Tool::Stack payload, int peer_id, bool wants_callback, bool collect_all = false) {
            payload.object["__event"] = Tool::StackValue(name);
            payload.object["__source_resource"] = Tool::StackValue(Manager::Resource::get_resource_from_vm(vm));
            std::shared_ptr<API::Promise::Instance> promise;
            if (wants_callback) {
                uint32_t serial = ++serial_counter;
                payload.object["__serial"] = Tool::StackValue(static_cast<double>(serial));
                if (collect_all) payload.object["__collect_all"] = Tool::StackValue(true);
                promise = API::Promise::make(vm);
                {
                    std::lock_guard lock(pending_remote_mutex);
                    #if defined(VSDK_Client)
                    int expected_peer = 1;
                    #else
                    int expected_peer = peer_id;
                    #endif
                    pending_remote[serial] = PendingRemote { promise, std::chrono::steady_clock::now(), expected_peer };
                }
            }
            #if defined(VSDK_Client)
                send_packet(payload, peer_id);
            #else
            // Queue behind any entity spawn RPCs still pending from this frame (they are sent via Core::enqueue),
            // so a remote event emitted right after creating an entity never reaches clients before the entity does
                Vital::Engine::Core::get_singleton() -> enqueue([payload, peer_id]() { send_packet(payload, peer_id); });
            #endif
            return promise;
        }

        static void sweep_pending_remote() {
            std::vector<std::shared_ptr<API::Promise::Instance>> timed_out;
            {
                std::lock_guard lock(pending_remote_mutex);
                auto now = std::chrono::steady_clock::now();
                for (auto it = pending_remote.begin(); it != pending_remote.end(); ) {
                    if (now - it -> second.created > std::chrono::milliseconds(remote_timeout_ms)) {
                        timed_out.push_back(it -> second.promise);
                        it = pending_remote.erase(it);
                    }
                    else ++it;
                }
            }
            if (timed_out.empty()) return;
            auto vm = Manager::Sandbox::get_singleton() -> get_vm();
            if (!vm) return;

            for (auto& promise : timed_out) {
                if (API::Promise::is_pending(promise)) API::Promise::settle(promise, API::Promise::State::Rejected, vm, 0, 0);
            }
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

        static void spawn_thread(Machine* vm, int exec_ref, int args_ref, std::shared_ptr<API::Promise::Instance> promise = nullptr, const std::string& env = "") {
            auto root_vm = vm -> get_root();
            vm -> get_raw_reference(exec_ref);
            auto instance = API::Thread::make(vm);
            if (!env.empty()) instance -> env = env;
            vm -> get_raw_reference(exec_ref);
            int n_args = vm -> push_args(args_ref);
            vm -> move(instance -> thread_vm, 1 + n_args);

            auto weak_promise = std::weak_ptr<API::Promise::Instance>(promise);
            instance -> thread_vm -> set_finish_hook([weak_promise, root_vm, instance](Machine* thread_vm, int nresults) {
                instance -> thread_state = nullptr;
                auto promise = weak_promise.lock();
                if (!API::Promise::is_pending(promise)) return;
                if (nresults < 0) API::Promise::settle(promise, API::Promise::State::Rejected, root_vm, 0, 0);
                else {
                    int base = root_vm -> get_count() + 1;
                    thread_vm -> move(root_vm, nresults);
                    API::Promise::settle(promise, API::Promise::State::Resolved, root_vm, base, nresults);
                }
            });
            API::Thread::safe_resume(instance, n_args);
        }

        // Exposes the authenticated transport sender to a handler as the `client` global of the environment the handler function itself runs in
        // (its own `_ENV` upvalue, which can differ from the environment that registered it, e.g. a function passed in from another resource).
        // `sender` > 0: remote-triggered (server build only), `client` = peer id. `sender` == 0: local emit, `client` = nil.
        // The previous value is restored on scope exit, so nested emits and a resource's own `client` global stay intact.
        // Note: for async handlers the value is only guaranteed until the handler's first yield - copy it into a local first.
        struct ClientScope {
            Machine* vm = nullptr;
            int env_ref = LUA_NOREF;
            int prev_ref = LUA_NOREF;
            bool active = false;

            ClientScope(Machine* vm, int exec_ref, int sender) : vm(vm) {
                const int base = vm -> get_count();
                vm -> get_raw_reference(exec_ref);
                if (!vm -> is_function(-1)) { vm -> pop(vm -> get_count() - base); return; }
                // Find the handler's own environment; a function that never touches globals has no _ENV upvalue and cannot read `client`
                bool found = false;
                for (int i = 1; ; ++i) {
                    const char* upname = vm -> get_upvalue(-1, i);
                    if (!upname) break;
                    if (std::strcmp(upname, "_ENV") == 0) { found = true; break; }
                    vm -> pop(1);
                }
                if (!found || !vm -> is_table(-1)) { vm -> pop(vm -> get_count() - base); return; }
                // Only resource environments are touched, never an arbitrary table
                vm -> push(-1);
                vm -> get_raw_table(LUA_REGISTRYINDEX);
                const bool registered = vm -> is_string(-1);
                vm -> pop(1);
                if (!registered) { vm -> pop(vm -> get_count() - base); return; }
                const int env_idx = vm -> get_count();
                vm -> push_string("client");
                vm -> get_raw_table(env_idx);
                const bool had_prev = !vm -> is_nil(-1);
                // Local emit with nothing to hide: leave the environment untouched
                if (sender <= 0 && !had_prev) { vm -> pop(vm -> get_count() - base); return; }
                prev_ref = vm -> set_raw_reference(-1); // LUA_REFNIL when there was no previous value
                vm -> pop(1);
                vm -> push_string("client");
                if (sender > 0) vm -> push_value(sender);
                else vm -> push_nil();
                vm -> set_raw_table(env_idx);
                env_ref = vm -> set_raw_reference(env_idx);
                vm -> pop(vm -> get_count() - base);
                active = true;
            }

            ~ClientScope() {
                if (!active) return;
                vm -> get_raw_reference(env_ref);
                const int env_idx = vm -> get_count();
                vm -> push_string("client");
                if (prev_ref == LUA_REFNIL || prev_ref == LUA_NOREF) vm -> push_nil();
                else vm -> get_raw_reference(prev_ref);
                vm -> set_raw_table(env_idx);
                vm -> pop(1);
                vm -> del_raw_reference(prev_ref);
                vm -> del_raw_reference(env_ref);
            }
        };

        static std::shared_ptr<API::Promise::Instance> fire_one(Machine* vm, const Handler& handler, int args_ref, FireMode mode, int sender = 0) {
            ClientScope client_scope(vm, handler.exec_ref, sender);
            std::shared_ptr<API::Promise::Instance> promise;
            if (mode == FireMode::EmitCallback) promise = API::Promise::make(vm);
            if (handler.async) {
                const int base = vm -> get_count();
                vm -> get_raw_reference(args_ref);
                const int orig = vm -> get_count();
                vm -> create_table();
                const int copy = vm -> get_count();
                int n = vm -> get_length(orig);
                for (int i = 1; i <= n; ++i) {
                    vm -> table_get_value(i, orig);
                    vm -> set_table_field(i, copy);
                }
                int args_ref_copy = vm -> set_raw_reference(copy);
                vm -> pop(vm -> get_count() - base);
                spawn_thread(vm, handler.exec_ref, args_ref_copy, promise, handler.env);
                vm -> del_raw_reference(args_ref_copy);
            }
            else {
                vm -> get_raw_reference(handler.exec_ref);
                int n_args = vm -> push_args(args_ref);
                if (mode == FireMode::EmitCallback) {
                    int base = vm -> get_count() - n_args - 1;
                    if (vm -> call(n_args)) {
                        int result_count = vm -> get_count() - base;
                        API::Promise::settle(promise, API::Promise::State::Resolved, vm, base + 1, result_count);
                    }
                    else API::Promise::settle(promise, API::Promise::State::Rejected, vm, 0, 0);
                }
                else vm -> call(n_args, 0);
            }
            return promise;
        }

        static void fire_all(Machine* vm, std::vector<std::pair<int, Handler>> snapshot, const std::string& name, int args_ref, FireMode mode, std::vector<std::shared_ptr<API::Promise::Instance>>* promises = nullptr, bool collect_all = false, int sender = 0) {
            std::vector<int> exhausted;
            ++dispatch_depth;
            for (auto& [ref, handler] : snapshot) {
                auto promise = fire_one(vm, handler, args_ref, mode, sender);
                if (promise && promises) promises -> push_back(promise);
                if (handler.subscription_limit > 0) bump_subscription(name, ref, exhausted);
                if (mode == FireMode::EmitCallback && !collect_all) break;
            }
            --dispatch_depth;
            sweep_exhausted(vm, name, exhausted);
        }

        static void settle_aggregate(std::shared_ptr<AggState> state) {
            Machine::enqueue([state]() {
                auto agg = state -> agg_promise.lock();
                if (!API::Promise::is_pending(agg)) return;

                auto root_vm = state -> vm;
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
                if (!API::Promise::is_pending(promise)) {
                    auto root_vm = vm -> get_root();
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

        static void dispatch_remote(Machine* vm, const Tool::Stack& payload) {
            auto reply_ptr = payload.get("__reply_serial");
            if (reply_ptr && reply_ptr -> is<double>()) {
                uint32_t serial = 0;
                int sender = 0;
                if (!to_serial(reply_ptr -> as<double>(), serial) || !get_sender(payload, sender)) return;
                std::shared_ptr<API::Promise::Instance> promise;
                {
                    std::lock_guard lock(pending_remote_mutex);
                    auto it = pending_remote.find(serial);
                    // Only the peer the request was sent to may settle it
                    if (it != pending_remote.end() && it -> second.peer == sender) {
                        promise = it -> second.promise;
                        pending_remote.erase(it);
                    }
                }
                if (!API::Promise::is_pending(promise)) return;

                auto root_vm = vm -> get_root();
                int base = root_vm -> get_count() + 1;
                int count = static_cast<int>(payload.array.size());
                for (auto& v : payload.array) root_vm -> push_value(v);
                auto error_ptr = payload.get("__reply_error");
                if (error_ptr && error_ptr -> is<bool>() && error_ptr -> as<bool>()) {
                    API::Promise::settle(promise, API::Promise::State::Rejected, root_vm, 0, 0);
                    return;
                }
                API::Promise::settle(promise, API::Promise::State::Resolved, root_vm, base, count);
                return;
            }

            auto event_name_ptr = payload.get("__event");
            if (!event_name_ptr || !event_name_ptr -> is<std::string>()) return;
            std::string name = event_name_ptr -> as<std::string>();
            #if defined(VSDK_Client)
            {
                auto src_ptr = payload.get("__source_resource");
                if (src_ptr && src_ptr -> is<std::string>()) {
                    std::string src = src_ptr -> as<std::string>();
                    if (!Manager::Resource::get_singleton() -> is_running(src)) {
                        std::lock_guard lock(pending_resource_remote_mutex);
                        pending_resource_remote[src].push_back(payload);
                        return;
                    }
                }
            }
            #endif

            auto serial_ptr = payload.get("__serial");
            bool wants_reply = serial_ptr && serial_ptr -> is<double>();
            uint32_t serial = 0;
            if (wants_reply && !to_serial(serial_ptr -> as<double>(), serial)) wants_reply = false;
            int reply_peer = 0;
            get_sender(payload, reply_peer);
            // Handlers see `client` only for packets received by the server; the client never sees a sender
            #if defined(VSDK_Client)
            const int client_peer = 0;
            #else
            const int client_peer = reply_peer;
            #endif
            auto collect_ptr = payload.get("__collect_all");
            bool collect_all = wants_reply && collect_ptr && collect_ptr -> is<bool>() && collect_ptr -> as<bool>();
            std::vector<std::pair<int, Handler>> snapshot;
            {
                std::lock_guard lock(buffer_mutex);
                auto it = buffer.find(name);
                if (it != buffer.end()) snapshot = it -> second.handlers;
            }

            int args_ref = store_stack_args(vm, payload.array);
            if (snapshot.empty()) {
                vm -> del_raw_reference(args_ref);
                if (wants_reply) send_reply(serial, reply_peer);
                return;
            }
            if (!wants_reply) {
                fire_all(vm, std::move(snapshot), name, args_ref, FireMode::Emit, nullptr, false, client_peer);
                vm -> del_raw_reference(args_ref);
                return;
            }

            std::vector<std::shared_ptr<API::Promise::Instance>> promises;
            fire_all(vm, std::move(snapshot), name, args_ref, FireMode::EmitCallback, &promises, collect_all, client_peer);
            vm -> del_raw_reference(args_ref);
            if (promises.empty()) return;

            auto agg = aggregate_promises(vm, promises);
            // Synchronous handlers settle their promise before we get here, and 'promise:settle' only fires for promises that are still pending - reply right away in that case
            if (!API::Promise::is_pending(agg)) {
                auto root_vm = vm -> get_root();
                Tool::Stack results;
                std::unordered_set<const void*> visited;
                for (int i = 1; i <= agg -> values; ++i) {
                    root_vm -> get_raw_reference(agg -> get_reference(agg -> value_reference(i)));
                    results.array.emplace_back(root_vm -> collect_value(root_vm -> get_count(), visited));
                    root_vm -> pop(1);
                }
                send_reply(serial, reply_peer, &results.array, agg -> state == API::Promise::State::Rejected);
                return;
            }
            std::weak_ptr<API::Promise::Instance> weak_agg = agg;
            register_reply_callback(agg, [serial, reply_peer, weak_agg](Machine* root_vm, const Tool::Stack& results) {
                auto settled = weak_agg.lock();
                send_reply(serial, reply_peer, &results.array, settled && settled -> state == API::Promise::State::Rejected);
            });
        }

        static void init(Machine* vm) {
            static bool initialized = false;
            if (initialized) return;
            initialized = true;

            Tool::Timer::create([](Tool::Timer*, int) {
                sweep_pending_remote();
            }, remote_sweep_interval_ms, 0);
            
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
                auto root_vm = Manager::Sandbox::get_singleton() -> get_vm();
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

            #if defined(VSDK_Client)
            Tool::Event::bind("resource:started", [](Tool::Stack args) {
                if (args.array.empty() || !args.array[0].is<std::string>()) return;
                std::string resource_name = args.array[0].as<std::string>();
                std::vector<Tool::Stack> queued;
                {
                    std::lock_guard lock(pending_resource_remote_mutex);
                    auto it = pending_resource_remote.find(resource_name);
                    if (it != pending_resource_remote.end()) {
                        queued = std::move(it -> second);
                        pending_resource_remote.erase(it);
                    }
                }
                if (queued.empty()) return;
                auto vm = Manager::Sandbox::get_singleton() -> get_vm();
                if (!vm) return;

                for (auto& packet : queued) {
                    dispatch_remote(vm, packet);
                }
            });

            Tool::Event::bind("resource:stopped", [](Tool::Stack args) {
                if (args.array.empty() || !args.array[0].is<std::string>()) return;
                std::string resource_name = args.array[0].as<std::string>();
                std::lock_guard lock(pending_resource_remote_mutex);
                pending_resource_remote.erase(resource_name);
            });

            Tool::Event::bind("environment:free", [](Tool::Stack) {
                std::lock_guard lock(pending_resource_remote_mutex);
                pending_resource_remote.clear();
            });
            #endif

            Tool::Event::bind("sandbox:signal", [](Tool::Stack args) {
                if (args.array.size() < 2) return;
                if (!args.array[0].is<std::string>() || !args.array[1].is<std::shared_ptr<Tool::Stack>>()) return;
                auto stack = args.array[1].as<std::shared_ptr<Tool::Stack>>();
                if (!stack) return;
                auto vm = Manager::Sandbox::get_singleton() -> get_vm();
                if (!vm) return;
            
                std::string name = args.array[0].as<std::string>();
                std::vector<std::pair<int, Handler>> snapshot;
                {
                    std::lock_guard lock(buffer_mutex);
                    auto it = buffer.find(name);
                    if (it != buffer.end()) snapshot = it -> second.handlers;
                }
                if (snapshot.empty()) return;

                int args_ref = store_stack_args(vm, stack -> array);
                fire_all(vm, std::move(snapshot), name, args_ref, FireMode::Emit);
                vm -> del_raw_reference(args_ref);
            });
        }

        static void bind(Machine* vm) {
            API::bind(vm, base_scope, "on", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, exec, config = nil)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_function)
                    .optional(3, &Machine::is_table);

                std::string name = vm -> get_string(1);
                Handler handler;
                handler.exec_ref = vm -> set_raw_reference(2);
                handler.env = vm -> get_environment_id();
                read_config(vm, 3, handler);
                {
                    std::lock_guard lock(buffer_mutex);
                    insert_ordered(buffer[name], handler.exec_ref, handler);
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
                    .require(1, &Machine::is_string)
                    .optional(2, &Machine::is_table);

                std::string name = vm -> get_string(1);
                auto opts = read_emit_options(vm);
                if (opts.remote) {
                    #if !defined(VSDK_Client)
                    // TODO: Improve logs?? Reason: event.emit, use base_name call etc to automate?
                    verify_peer("event.emit", opts.peer_id, false);
                    #endif
                    send_remote_emit(vm, name, vm -> collect_args(opts.args_start), opts.peer_id, false);
                }
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

            #if defined(VSDK_Benchmark)
            API::bind(vm, base_scope, "emit_native", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, ...)")
                    .require(1, &Machine::is_string);

                std::string name = vm -> get_string(1);
                Tool::Stack payload = vm -> collect_args(2);
                Vital::Engine::Core::get_singleton() -> emit_native(name, payload);
                vm -> push_value(true);
                return 1;
            });
            #endif
            
            API::bind(vm, base_scope, "emit_callback", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(name, options = nil, ...)")
                    .require(1, &Machine::is_string)
                    .optional(2, &Machine::is_table);

                std::string name = vm -> get_string(1);
                auto opts = read_emit_options(vm);
                if (opts.remote) {
                    #if !defined(VSDK_Client)
                    // A peer that isn't connected is skipped: the promise is rejected right away instead of throwing,
                    // so awaiting scripts get a failed result and keep running.
                    if (opts.peer_id > 0 && !Manager::Network::get_singleton() -> is_peer_valid(opts.peer_id)) {
                        Tool::print("warn", fmt::format("event.emit_callback: peer '{}' is not connected, skipped", opts.peer_id));
                        auto rejected = API::Promise::make(vm);
                        API::Promise::settle(rejected, API::Promise::State::Rejected, vm, 0, 0);
                        push_promise(vm, rejected);
                        return 1;
                    }
                    // TODO: Improve logs?? Reason: event.emit_callback, use base_name call etc to automate?
                    verify_peer("event.emit_callback", opts.peer_id, true);
                    #endif
                    auto promise = send_remote_emit(vm, name, vm -> collect_args(opts.args_start), opts.peer_id, true, opts.collect_all);
                    push_promise(vm, promise);
                    return 1;
                }

                std::vector<std::pair<int, Handler>> snapshot;
                {
                    std::lock_guard lock(buffer_mutex);
                    auto it = buffer.find(name);
                    if (it != buffer.end()) snapshot = it -> second.handlers;
                }

                int args_ref = vm -> store_args(opts.args_start);
                std::vector<std::shared_ptr<API::Promise::Instance>> promises;
                fire_all(vm, std::move(snapshot), name, args_ref, FireMode::EmitCallback, &promises, opts.collect_all);
                vm -> del_raw_reference(args_ref);
                auto agg = aggregate_promises(vm, promises);
                push_promise(vm, agg);
                return 1;
            });
        }

        static void clean(const std::string& env) {
            if (!Manager::Sandbox::has_singleton()) return;
            auto vm = Manager::Sandbox::get_singleton() -> get_vm();
            if (!vm) return;

            std::vector<int> refs_to_delete;
            {
                std::lock_guard lock(buffer_mutex);
                for (auto eit = buffer.begin(); eit != buffer.end(); ) {
                    auto& vec = eit -> second.handlers;
                    for (auto vit = vec.begin(); vit != vec.end(); ) {
                        if (vit -> second.env == env) {
                            refs_to_delete.push_back(vit -> second.exec_ref);
                            vit = vec.erase(vit);
                        }
                        else ++vit;
                    }
                    if (vec.empty()) eit = buffer.erase(eit);
                    else ++eit;
                }
            }
            if (refs_to_delete.empty()) return;

            auto do_delete = std::make_shared<std::function<void()>>();
            *do_delete = [refs_to_delete, do_delete]() {
                if (!Manager::Sandbox::has_singleton()) return;
                auto vm = Manager::Sandbox::get_singleton() -> get_vm();
                if (!vm) return;

                if (dispatch_depth > 0) {
                    auto fn = do_delete;
                    Machine::enqueue([fn]() { (*fn)(); });
                    return;
                }
                for (int ref : refs_to_delete) vm -> del_raw_reference(ref);
            };
            (*do_delete)();
        }
    };
}