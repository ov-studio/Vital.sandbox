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

        struct Instance {
            int  id {};
            std::string env;
            std::atomic<bool> destroyed { false };
            std::atomic<bool> sleeping  { false };
            std::atomic<bool> awaiting  { false };
            std::atomic<bool> vm_owned  { true  };
            Machine* vm        = nullptr;
            Machine* thread_vm = nullptr;
            void**   userdata  = nullptr;
            std::string reference()      const { return fmt::format("{}:{}", base_name, id); }
            std::string self_reference() const { return fmt::format("{}:{}:self", base_name, id); }
        };

        inline static std::mutex mutex;
        inline static std::unordered_map<int, std::shared_ptr<Instance>> buffer;
        inline static std::atomic<int> next_id { 1 };

        static std::shared_ptr<Instance> fetch_instance(int id) {
            std::lock_guard<std::mutex> lock(mutex);
            auto it = buffer.find(id);
            return it != buffer.end() ? it->second : nullptr;
        }

        static void clean_instance(std::shared_ptr<Instance> instance) {
            if (!instance) return;
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (buffer.find(instance->id) == buffer.end()) return;
                buffer.erase(instance->id);
            }
            instance->destroyed = true;

            if (instance->vm_owned.exchange(false)) {
                Machine* tvm = instance->thread_vm;
                instance->thread_vm = nullptr;
                if (tvm) delete tvm;
            } else {
                instance->thread_vm = nullptr;
            }

            if (instance->vm) {
                vm_module::release_userdata_ptr(instance->userdata);
                if (instance->vm->is_reference(instance->self_reference()))
                    instance->vm->del_reference(instance->self_reference());
                if (instance->vm->is_reference(instance->reference()))
                    instance->vm->del_reference(instance->reference());
                instance->vm = nullptr;
            }
        }

        static bool safe_resume(std::shared_ptr<Instance> instance, int args) {
            if (!instance || instance->destroyed) return false;
            if (!instance->vm_owned.load()) return false;
            if (!instance->thread_vm)       return false;

            if (!instance->vm) {
                Machine* tvm = instance->thread_vm;
                instance->thread_vm = nullptr;
                instance->vm_owned.store(false);
                if (tvm) delete tvm;
                clean_instance(instance);
                return false;
            }

            vm_state* raw_state = instance->thread_vm->get_state();
            if (!raw_state) return false;

            {
                int pre_status = lua_status(raw_state);
                if (pre_status != LUA_OK && pre_status != LUA_YIELD) {
                    instance->thread_vm = nullptr;
                    instance->vm_owned.store(false);
                    clean_instance(instance);
                    return false;
                }
            }

            instance->vm_owned.store(false);
            instance->thread_vm->resume(args);

            int status = lua_status(raw_state);
            if (status != LUA_YIELD) {
                instance->thread_vm = nullptr;
                clean_instance(instance);
                return false;
            }

            instance->vm_owned.store(true);
            return true;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Thread>(vm, base_name);

            Promise::register_resume_dispatcher(
                [](int thread_id, bool resolved,
                   const std::vector<Promise::SerialValue>& values) {

                    auto instance = Thread::fetch_instance(thread_id);
                    if (!instance || instance->destroyed) return;

                    if (!instance->vm_owned.load()) return;
                    if (!instance->thread_vm)       return;

                    lua_State* dst = instance->thread_vm->get_state();
                    if (!dst) return;

                    if (!Machine::fetch_machine(dst)) return;

                    lua_pushboolean(dst, resolved ? 1 : 0);
                    for (auto& v : values) v.push(dst);

                    instance->awaiting = false;
                    safe_resume(instance, 1 + static_cast<int>(values.size()));
                }
            );

            // thread.create(exec) -> thread userdata
            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(exec)")
                    .require(1, &Machine::is_function);

                std::string env = vm->get_environment_id();
                auto instance   = std::make_shared<Instance>();
                instance->id    = next_id.fetch_add(1);
                instance->env   = env;
                instance->vm    = vm;

                Machine* thread_vm  = vm->create_thread();
                instance->thread_vm = thread_vm;

                vm->set_reference(instance->reference(), 1);
                vm->pop(1);

                vm->create_object(base_name, instance.get());
                instance->userdata = vm_module::get_userdata_ptr(vm, -1);

                vm->set_reference(instance->self_reference(), -1);

                {
                    std::lock_guard<std::mutex> lock(mutex);
                    buffer[instance->id] = instance;
                }

                vm->get_reference(instance->self_reference(), true);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            // thread:resume()
            vm_module::bind_method<Instance>(vm, base_name, "resume", [](auto vm, auto self, auto& id) -> int {
                if (!self || self->destroyed || !self->thread_vm) { vm->push_value(false); return 1; }

                auto instance = fetch_instance(self->id);
                if (!instance) { vm->push_value(false); return 1; }

                self->vm->get_reference(self->reference(),      true);
                self->vm->move(self->thread_vm, 1);

                self->vm->get_reference(self->self_reference(), true);
                self->vm->move(self->thread_vm, 1);

                safe_resume(instance, 1);
                vm->push_value(true);
                return 1;
            });

            // thread:destroy()
            vm_module::bind_method<Instance>(vm, base_name, "destroy", [](auto vm, auto self, auto& id) -> int {
                if (!self || self->destroyed) { vm->push_value(false); return 1; }
                auto instance = fetch_instance(self->id);
                if (instance) clean_instance(instance);
                vm_module::release_userdata(vm, 1);
                vm->push_value(true);
                return 1;
            });

            // thread:is_instance()
            vm_module::bind_method<Instance>(vm, base_name, "is_instance", [](auto vm, auto self, auto& id) -> int {
                vm->push_value(self && !self->destroyed);
                return 1;
            });

            // thread:get_thread() -> bool
            vm_module::bind_method<Instance>(vm, base_name, "get_thread", [](auto vm, auto self, auto& id) -> int {
                if (!self || self->destroyed || !self->thread_vm) { vm->push_value(false); return 1; }
                vm->push_value(self->thread_vm->get_state() == vm->get_state());
                return 1;
            });

            // thread:pause()
            vm_module::bind_method<Instance>(vm, base_name, "pause", [](auto vm, auto self, auto& id) -> int {
                if (!self || self->destroyed || !vm->is_virtual()) { vm->push_value(false); return 1; }
                return lua_yieldk(vm->get_state(), 0, 0,
                    [](lua_State*, int, lua_KContext) -> int { return 0; });
            });

            // thread:sleep(duration)
            vm_module::bind_method<Instance>(vm, base_name, "sleep", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(duration)")
                    .require(2, &Machine::is_number)
                    .validate(2, [](auto vm, int index) { return vm->get_int(index) >= 0; }, "expected >= 0");

                if (!self || self->destroyed || self->sleeping) { vm->push_value(false); return 1; }

                int duration   = vm->get_int(2);
                self->sleeping = true;

                auto instance = fetch_instance(self->id);
                auto weak     = std::weak_ptr<Instance>(instance);

                Tool::Timer::create([weak](Tool::Timer*, int) {
                    auto instance = weak.lock();
                    if (!instance || instance->destroyed) return;
                    instance->sleeping = false;
                    if (!instance->vm_owned.load() || !instance->thread_vm) return;
                    safe_resume(instance, 0);
                }, duration, 1);

                return lua_yieldk(vm->get_state(), 0, 0,
                    [](lua_State*, int, lua_KContext) -> int { return 0; });
            });

            // thread:await(promise) -> resolved_bool, value1, value2, ...
            vm_module::bind_method<Instance>(vm, base_name, "await", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(promise)")
                    .require(2, [](Machine* vm, int index) {
                        return vm_module::is_userdata(vm, Promise::base_name, index);
                    });

                if (!self || self->destroyed || self->awaiting) { vm->push_value(false); return 1; }
                if (!vm->is_virtual()) { vm->push_value(false); return 1; }

                auto ud = static_cast<Promise::Instance**>(lua_touserdata(vm->get_state(), 2));
                if (!ud || !*ud) { vm->push_value(false); return 1; }
                auto promise_inst = Promise::fetch_instance((*ud)->id);
                if (!promise_inst) { vm->push_value(false); return 1; }

                if (promise_inst->state != Promise::State::Pending) {
                    lua_State* dst = vm->get_state();
                    lua_pushboolean(dst, promise_inst->resolved ? 1 : 0);
                    for (auto& v : promise_inst->serial_values) v.push(dst);
                    return 1 + static_cast<int>(promise_inst->serial_values.size());
                }

                self->awaiting = true;
                promise_inst->waiting.push_back({ self->id });

                int base = lua_gettop(vm->get_state());
                lua_KContext ctx = (static_cast<lua_KContext>(base)      << 16)
                                 | (static_cast<lua_KContext>(self->id)  & 0xFFFF);

                return lua_yieldk(vm->get_state(), 0, ctx,
                    [](lua_State* L, int, lua_KContext ctx) -> int {
                        int base      = static_cast<int>((ctx >> 16) & 0xFFFF);
                        int thread_id = static_cast<int>( ctx        & 0xFFFF);
                        auto inst = Thread::fetch_instance(thread_id);
                        if (inst) inst->awaiting = false;
                        int n = lua_gettop(L) - base;
                        return n > 0 ? n : 0;
                    }
                );
            });
        }

        static void clean(const std::string& env) {
            std::vector<std::shared_ptr<Instance>> to_clean;
            {
                std::lock_guard<std::mutex> lock(mutex);
                for (auto& [id, instance] : buffer)
                    if (instance->env == env) to_clean.push_back(instance);
            }
            for (auto& instance : to_clean) instance->destroyed = true;
            for (auto& instance : to_clean) clean_instance(instance);
        }
    };
}