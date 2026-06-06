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

        // Push args from stored table onto stack using a stable absolute index.
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

        // Push a promise's userdata onto 'vm' (which may be a coroutine Machine).
        // promise->vm is always the root vm; its Lua registry refs are shared across
        // the entire lua_State, so lua_rawgeti on any coroutine's state with the same
        // ref integer correctly retrieves the value onto that coroutine's stack.
        static void push_promise(Machine* vm, std::shared_ptr<Promise::Instance> promise) {
            int ref = promise->get_reference(promise->self_reference());
            vm->get_raw_reference(ref);
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

        // Spawn a Thread::Instance coroutine for an async event handler.
        //
        // Bug 1 fixed — create_thread on root_vm, not calling vm:
        //   If emit_callback is called from inside a coroutine (e.g. inside thread.create),
        //   'vm' is that coroutine's Machine.  vm->create_thread() makes the new handler
        //   thread a child of the coroutine Machine.  When the coroutine finishes, ~Machine
        //   destroys all children — killing the sleeping handler thread, nulling its
        //   lua_State*, and crashing when the sleep timer fires.  Using root_vm->create_thread()
        //   ties the handler thread's lifetime to the sandbox session instead.
        //
        // Bug 2 fixed — anchor the coroutine (lua_State*), not the userdata:
        //   After store() pushes the thread userdata, -1 is the userdata.  The coroutine
        //   is one slot below at the saved absolute index.  Anchoring -1 (userdata) left
        //   the coroutine unregistered and eligible for GC.  When collected, the machines
        //   map held a dangling key and the next map operation crashed.
        static void spawn_thread(Machine* vm,
                                  int exec_ref,
                                  int args_ref,
                                  std::shared_ptr<Promise::Instance> promise = nullptr) {
            auto* root_vm = vm->get_root();
            auto  instance = Thread::Instance::init(vm);

            // Create the handler coroutine under root_vm so its lifetime is independent
            // of whichever coroutine is calling spawn_thread.
            auto thread_vm = root_vm->create_thread();
            instance->thread_vm = thread_vm;

            // Save the coroutine's absolute index on root_vm BEFORE anything else is
            // pushed there — used below to anchor it in the Lua registry.
            int coroutine_idx = root_vm->get_count();

            // Push handler fn + args on the calling vm's stack, then move to thread_vm.
            push_ref(vm, exec_ref);
            int n_args = push_args_ref(vm, args_ref);
            vm->move(thread_vm, 1 + n_args);

            // store() pushes the Thread Instance userdata onto vm (calling stack).
            instance->store();
            int self_ref = root_vm->get_reference("runtime", instance->self_reference());

            // Inject eventthread global into the coroutine
            thread_vm->get_raw_reference(self_ref);
            lua_setglobal(thread_vm->get_state(), "eventthread");

            // Anchor the COROUTINE on root_vm at its saved absolute index.
            // root_vm->-1 is still the coroutine because store() pushed the userdata
            // onto 'vm' (the calling stack), not root_vm.
            instance->set_reference(instance->thread_reference(), coroutine_idx);
            root_vm->pop(1); // pop coroutine from root_vm (registry keeps it alive)

            vm->pop(1); // pop thread Instance userdata from calling vm

            if (promise) {
                auto weak_promise = std::weak_ptr<Promise::Instance>(promise);
                instance->on_finish = [weak_promise, root_vm](std::shared_ptr<Thread::Instance>) {
                    auto p = weak_promise.lock();
                    if (!p) return;
                    root_vm->push_nil();
                    Promise::settle(p, Promise::State::Resolved, root_vm, root_vm->get_count(), 1);
                    root_vm->pop(1);
                };
            }

            Thread::safe_resume(instance, n_args);
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
                        vm->pop(1); // pop ud left by Instance::store()
                        if (promises) promises->push_back(promise);
                    }
                    int args_ref_copy = store_args_ref_copy(vm, args_ref);
                    spawn_thread(vm, h.exec_ref, args_ref_copy, promise);
                    free_ref(vm, args_ref_copy);
                } else {
                    if (mode == FireMode::EmitCallback) {
                        auto promise = Promise::make(vm);
                        vm->pop(1); // pop ud left by Instance::store()
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


        // ----------------------------------------------------------------
        // Bind
        // ----------------------------------------------------------------
        static void bind(Machine* vm) {

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

            // emit_callback: push promise userdata onto the CALLING vm's stack.
            // promise->vm is root_vm, but Lua registry refs (luaL_ref integers) are
            // global to the lua_State and valid on any coroutine's stack via lua_rawgeti.
            // Using push_promise() instead of instance->get_reference(..., true) avoids
            // pushing onto root_vm when the caller is a coroutine (which would leave the
            // return value on the wrong stack and return garbage to Lua).
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
                    vm->push_nil();
                    Promise::settle(promise, Promise::State::Resolved, vm, vm->get_count(), 1);
                    vm->pop(1);
                    push_promise(vm, promise); // push onto calling vm ✓
                    return 1;
                }

                int args_ref = store_args_ref(vm, 2);

                std::vector<std::shared_ptr<Promise::Instance>> promises;
                fire_all(vm, std::move(snapshot), name, args_ref, FireMode::EmitCallback, &promises);

                free_ref(vm, args_ref);

                if (promises.size() == 1) {
                    push_promise(vm, promises[0]); // push onto calling vm ✓
                } else {
                    vm->create_table();
                    for (int i = 0; i < static_cast<int>(promises.size()); ++i) {
                        push_promise(vm, promises[i]);
                        vm->set_table_field(i + 1, -2);
                    }
                }

                return 1;
            });

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