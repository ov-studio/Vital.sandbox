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
        };
        inline static std::mutex mutex;
        inline static std::unordered_map<int, std::shared_ptr<Instance>> buffer;
        inline static std::atomic<int> next_id { 1 };

        static void clean_instance(std::shared_ptr<Instance> instance) {
            if (!Instance::erase(instance)) return;
            if (instance -> vm_owned.exchange(false)) {
                auto tvm = instance -> thread_vm;
                instance -> thread_vm = nullptr;
                if (tvm) delete tvm;
            }
            else instance -> thread_vm = nullptr;
            Instance::release(instance);
        }

        static bool safe_resume(std::shared_ptr<Instance> instance, int args) {
            if (!instance || instance -> destroyed || !instance -> vm_owned.load() || !instance -> thread_vm) return false;
            if (!instance -> vm) {
                auto tvm = instance -> thread_vm;
                instance -> thread_vm = nullptr;
                instance -> vm_owned.store(false);
                if (tvm) delete tvm;
                clean_instance(instance);
                return false;
            }

            vm_state* raw_state = instance -> thread_vm -> get_state();
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
            instance -> vm_owned.store(false);
            instance -> thread_vm -> resume(args);
            if (lua_status(raw_state) != LUA_YIELD) {
                instance -> thread_vm = nullptr;
                clean_instance(instance);
                return false;
            }
            instance -> vm_owned.store(true);
            return true;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Thread>(vm, base_name);

            Promise::register_resume_dispatcher([](int thread_id, bool resolved, std::shared_ptr<Promise::Instance> promise) {
                auto instance = Instance::find(thread_id);
                if (!instance || instance -> destroyed || !instance -> vm_owned.load() || !instance -> thread_vm) return;                
                instance -> thread_vm -> push_bool(resolved);
                int value_count = Promise::push_values(promise, instance -> thread_vm);
                instance -> awaiting = false;
                safe_resume(instance, 1 + value_count);
            });

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(exec)")
                    .require(1, &Machine::is_function);

                auto instance  = Instance::init(vm);
                auto thread_vm = vm -> create_thread();
                instance -> thread_vm = thread_vm;
                instance -> set_ref(instance -> reference(), 1);
                vm -> pop(1);
                vm -> create_object(base_name, instance.get());
                instance -> userdata = vm_module::get_userdata_ptr(vm, -1);
                instance -> set_ref(instance -> self_reference(), -1);
                Instance::store(instance);
                vm -> get_reference(instance -> self_reference(), true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, base_name, "resume", [](auto vm, auto self, auto& id) -> int {
                if (!self -> thread_vm || self -> sleeping || self -> awaiting) vm -> push_value(false);
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

            vm_module::bind_method<Instance>(vm, base_name, "destroy", [](auto vm, auto self, auto& id) -> int {
                return Instance::destroy(vm);
            });

            // TODO: SHOULD RETRIEVE RUNNING THREAD INSTEAD call it running
            vm_module::bind_method<Instance>(vm, base_name, "get_thread", [](auto vm, auto self, auto& id) -> int {
                if (!self -> thread_vm) vm -> push_value(false);
                else vm -> push_value(self -> thread_vm -> get_state() == vm -> get_state());
                return 1;
            });

            // TODO: FROM HERE EDIT CONDTNL BLOCKS
            vm_module::bind_method<Instance>(vm, base_name, "pause", [](auto vm, auto self, auto& id) -> int {
                if (!vm -> is_virtual()) { 
                    vm -> push_value(false); 
                    return 1;
                }
                return lua_yieldk(vm -> get_state(), 0, 0, [](lua_State*, int, lua_KContext) -> int { return 0; });
            });

            vm_module::bind_method<Instance>(vm, base_name, "sleep", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(duration)")
                    .require(2, &Machine::is_number)
                    .validate(2, [](auto vm, int index) { return vm -> get_int(index) >= 0; }, "expected >= 0");

                if (self -> sleeping || self -> awaiting) {
                    vm -> push_value(false);
                    return 1;
                }

                int duration = vm -> get_int(2);
                self -> sleeping = true;
                auto weak = std::weak_ptr<Instance>(self);
                Tool::Timer::create([weak](Tool::Timer*, int) {
                    auto self = weak.lock();
                    if (!self || self -> destroyed) return;
                    self -> sleeping = false;
                    if (!self -> vm_owned.load() || !self -> thread_vm) return;
                    safe_resume(self, 0);
                }, duration, 1);
                return lua_yieldk(vm -> get_state(), 0, 0, [](lua_State*, int, lua_KContext) -> int { return 0; });
            });

            vm_module::bind_method<Instance>(vm, base_name, "await", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(promise)")
                    .require(2, [](Machine* vm, int index) {
                        return vm_module::is_userdata(vm, Promise::base_name, index);
                    });

                if (!vm -> is_virtual() || self -> sleeping || self -> awaiting) {
                    vm -> push_value(false);
                    return 1;
                }

                auto ud = static_cast<Promise::Instance**>(vm -> get_userdata(2));
                if (!ud || !*ud) { vm -> push_value(false); return 1; }
                auto promise = Promise::Instance::find((*ud) -> id);
                if (!promise || promise -> destroyed) { vm -> push_value(false); return 1; }

                if (promise -> state != Promise::State::Pending) {
                    vm -> push_bool(promise -> resolved);
                    return 1 + Promise::push_values(promise, vm);
                }
                self -> awaiting = true;
                promise -> waiting.push_back(self -> id);

                struct AwaitCTX { int base; int thread_id; };
                auto actx = new AwaitCTX { vm -> get_count(), self -> id };
                lua_KContext ctx = reinterpret_cast<lua_KContext>(actx);
                return lua_yieldk(vm -> get_state(), 0, ctx, [](lua_State* L, int, lua_KContext ctx) -> int {
                    auto actx = reinterpret_cast<AwaitCTX*>(ctx);
                    auto self = Instance::find(actx -> thread_id);
                    if (self) self -> awaiting = false;
                    int n = lua_gettop(L) - actx -> base;
                    delete actx;
                    return n > 0 ? n : 0;
                });
            });
        }

        static void clean(const std::string& env) {
            vm_module::collect_env<Instance>(mutex, buffer, env, clean_instance, true);
        }
    };
}
