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
            std::string thread_reference() const { return fmt::format("vm_instance:{}:{}:thread", Owner::base_name, id); }
        };
        inline static vm_registry<Instance> registry;

        static void clean_instance(std::shared_ptr<Instance> instance) {
            if (!Instance::erase(instance)) return;
            if (instance -> vm_owned.exchange(false)) {
                auto thread_vm = instance -> thread_vm;
                instance -> thread_vm = nullptr;
                if (thread_vm) delete thread_vm;
            }
            else instance -> thread_vm = nullptr;
            Instance::release(instance);
        }

        static bool safe_resume(std::shared_ptr<Instance> instance, int args) {
            if (!instance || instance -> destroyed || !instance -> vm_owned.load() || !instance -> thread_vm) return false;
            if (!instance -> vm) {
                auto thread_vm = instance -> thread_vm;
                instance -> thread_vm = nullptr;
                instance -> vm_owned.store(false);
                if (thread_vm) delete thread_vm;
                clean_instance(instance);
                return false;
            }

            auto raw_state = instance -> thread_vm -> get_state();
            if (!raw_state) return false;
            {
                int status = lua_status(raw_state);
                if (status != LUA_OK && status != LUA_YIELD) {
                    instance -> thread_vm = nullptr;
                    instance -> vm_owned.store(false);
                    clean_instance(instance);
                    return false;
                }
            }
            auto thread_vm = instance -> thread_vm;
            instance -> thread_vm = nullptr;
            instance -> vm_owned.store(false);
            if (!thread_vm -> resume(args)) {
                clean_instance(instance);
                return false;
            }
            instance -> thread_vm = thread_vm;
            instance -> vm_owned.store(true);
            return true;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Thread>(vm, base_name);

            Promise::register_resume_dispatcher([](int thread_id, bool resolved, std::shared_ptr<Promise::Instance> promise) {
                Machine::enqueue([thread_id, resolved, promise]() {
                    auto instance = Instance::find(thread_id);
                    if (!instance || instance -> destroyed || !instance -> vm_owned.load() || !instance -> thread_vm) return;
                    instance -> thread_vm -> push_bool(resolved);
                    int value_count = Promise::push_values(promise, instance -> thread_vm);
                    instance -> awaiting = false;
                    safe_resume(instance, 1 + value_count);
                });
            });

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(exec)")
                    .require(1, &Machine::is_function);

                auto instance  = Instance::init(vm);
                auto thread_vm = vm -> create_thread();
                instance -> thread_vm = thread_vm;
                instance -> set_ref(instance -> thread_reference(), 2);
                instance -> set_ref(instance -> reference(), 1);
                vm -> pop(2);
                Instance::store(instance);
                Instance::bind(vm, base_name, instance);
                vm -> get_reference(instance -> self_reference(), true);
                return 1;
            });

            API::bind(vm, {base_name}, "current", [](auto vm, auto& id) -> int {
                std::shared_ptr<Instance> found;
                {
                    std::lock_guard<std::mutex> lock(Thread::registry.mutex);
                    for (auto& [id, instance] : Thread::registry.buffer) {
                        if (instance -> destroyed || !instance -> thread_vm) continue;
                        if (instance -> thread_vm -> get_state() == vm -> get_state()) {
                            found = instance;
                            break;
                        }
                    }
                }
                if (!found) vm -> push_value(false);
                else found -> vm -> get_reference(found -> self_reference(), true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, base_name, "resume", [](auto vm, auto self, auto& id) -> int {
                if (self -> sleeping || self -> awaiting) vm -> push_value(false);
                else {
                    self -> vm -> get_reference(self -> reference(), true);
                    self -> vm -> move(self -> thread_vm, 1);
                    self -> vm -> get_reference(self -> self_reference(), true);
                    self -> vm -> move(self -> thread_vm, 1);
                    safe_resume(self, 1);
                    vm -> push_value(true);
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "pause", [](auto vm, auto self, auto& id) -> int {
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

            vm_module::bind_method<Instance>(vm, base_name, "sleep", [](auto vm, auto self, auto& id) -> int {
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
                        auto captured = weak;
                        Machine::enqueue([captured]() {
                            auto self = captured.lock();
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

            vm_module::bind_method<Instance>(vm, base_name, "await", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(promise)")
                    .require(2, [](Machine* vm, int index) { return vm_module::is_userdata<Promise::Instance>(vm, Promise::base_name, index); });

                auto promise = vm_module::get_userdata_object<Promise::Instance>(vm, 2);
                if (!vm -> is_virtual() || self -> sleeping || self -> awaiting || !promise) {
                    vm -> push_value(false);
                    return 1;
                }
                else if (promise -> state != Promise::State::Pending) {
                    vm -> push_bool(promise -> resolved);
                    return 1 + Promise::push_values(promise, vm);
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