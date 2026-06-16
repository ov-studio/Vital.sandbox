/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: thread.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Thread APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/API/utility/promise.h>


/////////////////////////
// Vital: API: Thread //
/////////////////////////

namespace Vital::Sandbox::API {
    struct Thread : vm_module {
        inline static const std::string base_name = "thread";

        struct Instance : vm_instance<Instance> {
            using Owner = Thread;
            std::atomic<bool> sleeping { false };
            std::atomic<bool> awaiting { false };
            std::atomic<bool> vm_owned { true };
            Machine* thread_vm = nullptr;
            vm_state* thread_state = nullptr;

            bool is_alive() const {
                return thread_state ? true : false;
            }

            void clean_thread() {
                auto tvm = thread_vm;
                thread_vm = nullptr;
                thread_state = nullptr;
                if (tvm) delete tvm;
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> vm_owned.exchange(false)) instance -> clean_thread();
                else {
                    instance -> thread_vm = nullptr;
                    instance -> thread_state = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static std::shared_ptr<Instance> make(Machine* vm, bool push_to_stack = false) {
            auto instance = Instance::init(vm);
            auto thread_vm = vm -> create_thread();
            instance -> thread_vm = thread_vm;
            instance -> thread_state = thread_vm -> get_state();
            instance -> set_reference(instance -> value_reference("exec"), 1);
            instance -> set_reference(instance -> value_reference("thread"), 2);
            vm -> pop(2);
            instance -> store(push_to_stack);
            return instance;
        }

        static bool safe_resume(std::shared_ptr<Instance> instance, int args) {
            if (!instance || instance -> destroyed || !instance -> vm_owned.load() || !instance -> thread_vm) return false;
            if (!instance -> vm) {
                instance -> vm_owned.store(false);
                instance -> clean_thread();
                instance -> clean();
                return false;
            }

            auto raw_state = instance -> thread_vm -> get_state();
            if (!raw_state) return false;
            {
                int status = lua_status(raw_state);
                if (status != LUA_OK && status != LUA_YIELD) {
                    instance -> vm_owned.store(false);
                    instance -> clean_thread();
                    instance -> clean();
                    return false;
                }
            }

            auto thread_vm = instance -> thread_vm;
            instance -> thread_vm = nullptr;
            instance -> vm_owned.store(false);
            if (!thread_vm -> resume(args)) {
                instance -> thread_state = nullptr;
                instance -> clean();
                return false;
            }
            instance -> thread_vm = thread_vm;
            instance -> vm_owned.store(true);
            return true;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Thread>(vm);

            API::Promise::register_resume_dispatcher([](int thread_id, bool resolved, std::shared_ptr<API::Promise::Instance> promise) {
                if (thread_id == -1) {
                    int pid = promise -> id;
                    Machine::enqueue([pid, promise]() {
                        Tool::Event::emit("sandbox:reply", Tool::Stack({
                            Tool::StackValue(pid),
                            Tool::StackValue(promise)
                        }));
                    });
                }
                else {
                    Machine::enqueue([thread_id, resolved, promise]() {
                        auto instance = Instance::find(thread_id);
                        if (!instance || instance -> destroyed || !instance -> vm_owned.load() || !instance -> thread_vm) return;
                        instance -> thread_vm -> push_bool(resolved);
                        int values = API::Promise::push_values(promise, instance -> thread_vm);
                        instance -> awaiting = false;
                        safe_resume(instance, 1 + values);
                    });
                }
            });

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(exec)")
                    .require(1, &Machine::is_function);

                Thread::make(vm, true);
                return 1;
            });

            API::bind(vm, {base_name}, "current", [](auto vm, auto& id) -> int {
                std::shared_ptr<Instance> found;
                {
                    std::lock_guard<std::mutex> lock(Thread::registry.mutex);
                    for (auto& [id, instance] : Thread::registry.buffer) {
                        if (!Instance::find_unlocked(instance)) continue;
                        if (instance -> thread_state == vm -> get_state()) {
                            found = instance;
                            break;
                        }
                    }
                }
                if (!found) vm -> push_value(false);
                else found -> push_self(vm);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "resume", [](auto vm, auto self, auto& id) -> int {
                if (self -> sleeping || self -> awaiting) vm -> push_value(false);
                else {
                    self -> get_reference(self -> value_reference("exec"),   true);
                    self -> vm -> move(self -> thread_vm, 1);
                    self -> push_self(self -> vm);
                    self -> vm -> move(self -> thread_vm, 1);
                    safe_resume(self, 1);
                    vm -> push_value(true);
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "pause", [](auto vm, auto self, auto& id) -> int {
                if (!vm -> is_virtual()) {
                    vm -> push_value(false);
                    return 1;
                }
                else {
                    return lua_yieldk(vm -> get_state(), 0, 0, [](lua_State* state, int, lua_KContext) -> int {
                        lua_pushboolean(state, true);
                        return 1;
                    });
                }
            });

            vm_module::bind_method<Instance>(vm, "sleep", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(duration)")
                    .require(2, &Machine::is_number)
                    .validate(2, [](auto vm, int index) { return vm -> get_int(index) >= 0; }, "expected >= 0");

                if (!vm -> is_virtual() || self -> sleeping || self -> awaiting) {
                    vm -> push_value(false);
                    return 1;
                }
                else {
                    self -> sleeping = true;
                    int duration = vm -> get_int(2);
                    
                    auto weak = std::weak_ptr<Instance>(self);
                    Tool::Timer::create([weak](Tool::Timer*, int) {
                        Machine::enqueue([weak]() {
                            auto self = weak.lock();
                            if (!self || self -> destroyed) return;
                            self -> sleeping = false;
                            if (!self -> vm_owned.load() || !self -> thread_vm) return;
                            safe_resume(self, 0);
                        });
                    }, duration, 1);

                    return lua_yieldk(vm -> get_state(), 0, 0, [](lua_State* state, int, lua_KContext) -> int {
                        lua_pushboolean(state, true);
                        return 1;
                    });
                }
            });

            vm_module::bind_method<Instance>(vm, "await", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(promise)")
                    .require(2, [](Machine* vm, int index) { return vm_module::is_userdata<API::Promise::Instance>(vm, index); });

                auto promise = vm_module::get_userdata_object<API::Promise::Instance>(vm, 2);
                if (!vm -> is_virtual() || self -> sleeping || self -> awaiting || !promise) {
                    vm -> push_value(false);
                    return 1;
                }
                else if (promise -> state != API::Promise::State::Pending) {
                    vm -> push_bool(promise -> resolved);
                    return 1 + API::Promise::push_values(promise, vm);
                }
                else {
                    self -> awaiting = true;
                    promise -> waiting.push_back(self -> id);
                    struct AwaitCTX { int base; int thread_id; };
                    auto actx = new AwaitCTX { vm -> get_count(), self -> id };
                    lua_KContext ctx = reinterpret_cast<lua_KContext>(actx);

                    return lua_yieldk(vm -> get_state(), 0, ctx, [](lua_State* state, int, lua_KContext ctx) -> int {
                        auto actx = reinterpret_cast<AwaitCTX*>(ctx);
                        auto self = Instance::find_unlocked(actx -> thread_id);
                        if (self) self -> awaiting = false;
                        int n = lua_gettop(state) - actx -> base;
                        delete actx;
                        return n > 0 ? n : 0;
                    });
                }
            });
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}