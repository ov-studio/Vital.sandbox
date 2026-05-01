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

        // ---------------------------------------------------------------
        // clean_instance
        // Removes the instance from the buffer and releases all resources.
        //
        // thread_vm lifetime rule:
        //   Machine::resume() calls `delete this` when the coroutine
        //   finishes (non-LUA_YIELD). safe_resume() nulls thread_vm BEFORE
        //   calling clean_instance() in that case, so the guard below
        //   prevents a double-free.
        //   On resource teardown (Thread::clean), the coroutine is still
        //   suspended — thread_vm is alive and we delete it here.
        //   In all cases: only one code path deletes thread_vm.
        // ---------------------------------------------------------------
        static void clean_instance(std::shared_ptr<Instance> instance) {
            if (!instance) return;
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (buffer.find(instance->id) == buffer.end()) return;
                buffer.erase(instance->id);
            }
            instance->destroyed = true;

            if (instance->thread_vm) {
                delete instance->thread_vm;
                instance->thread_vm = nullptr;
            }

            if (instance->vm) {
                vm_module::release_userdata_ptr(instance->userdata);
                instance->vm->del_reference(instance->self_reference());
                instance->vm->del_reference(instance->reference());
                instance->vm = nullptr;
            }
        }

        // ---------------------------------------------------------------
        // safe_resume
        // The single resume entry point for all callers (sleep timer,
        // promise dispatcher, initial resume).
        //
        // Captures the raw lua_State* before calling Machine::resume()
        // because Machine::resume() calls `delete this` on non-LUA_YIELD,
        // making instance->thread_vm a dangling pointer immediately after.
        // ---------------------------------------------------------------
        static bool safe_resume(std::shared_ptr<Instance> instance, int args) {
            if (!instance || instance->destroyed || !instance->thread_vm) return false;
            if (!instance->vm) {
                // Owner vm already gone (resource teardown race) — delete the
                // coroutine Machine and clean up without touching any Lua state.
                delete instance->thread_vm;
                instance->thread_vm = nullptr;
                clean_instance(instance);
                return false;
            }

            vm_state* thread_state = instance->thread_vm->get_state();
            instance->thread_vm->resume(args);
            // thread_vm may now be deleted (Machine::resume called `delete this`)

            int status = lua_status(thread_state);
            if (status != LUA_YIELD) {
                if (status != LUA_OK) {
                    const char* raw = lua_tostring(thread_state, -1);
                    API::log(std::string(Tool::Log::error::label),
                             raw ? raw : "(unknown error in thread)");
                    lua_pop(thread_state, 1);
                }
                // Machine::resume() deleted thread_vm — null it before clean_instance
                // so the guard in clean_instance skips the already-freed pointer.
                instance->thread_vm = nullptr;
                clean_instance(instance);
                return false;
            }
            return true;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Thread>(vm, base_name);

            // ------------------------------------------------------------------
            // Register the promise-to-thread resume dispatcher.
            // Called once per Machine construction — the lambda only closes over
            // Thread statics so overwriting on repeated calls is harmless.
            // promise.h calls this function pointer from push_deferred with plain
            // serialised values; it never accesses Thread internals directly.
            // ------------------------------------------------------------------
            Promise::register_resume_dispatcher(
                [](int thread_id, bool resolved,
                   const std::vector<Promise::SerialValue>& values) {

                    auto instance = Thread::fetch_instance(thread_id);
                    if (!instance || instance->destroyed || !instance->thread_vm) return;

                    lua_State* dst = instance->thread_vm->get_state();

                    // Push (resolved_bool, v1, v2, ...) directly onto the coroutine
                    // stack. All values are plain C++ — no cross-state xmove needed.
                    lua_pushboolean(dst, resolved ? 1 : 0);
                    for (auto& v : values) v.push(dst);

                    // Clear awaiting before resume so the flag is correct if the
                    // coroutine calls await() again immediately.
                    instance->awaiting = false;

                    int n = static_cast<int>(values.size());
                    // safe_resume handles post-resume cleanup (null thread_vm,
                    // remove from buffer, release refs). Do NOT call
                    // instance->thread_vm->resume() directly here.
                    safe_resume(instance, 1 + n);
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

                Machine* thread_vm = vm->create_thread();
                instance->thread_vm = thread_vm;
                // lua_newthread pushed the coroutine object onto the owner stack —
                // pop it now that it is wrapped in a Machine*.
                vm->pop(1);

                // Store exec function ref (stack-neutral: push+luaL_ref).
                vm->set_reference(instance->reference(), 1);

                // Create the userdata and store a full-userdata ref for resume().
                // Light userdata cannot hold a metatable so we must use the full
                // userdata box to pass `self` into the exec function.
                vm->create_object(base_name, instance.get());
                instance->userdata = vm_module::get_userdata_ptr(vm, -1);
                vm->set_reference(instance->self_reference(), -1); // stack-neutral

                {
                    std::lock_guard<std::mutex> lock(mutex);
                    buffer[instance->id] = instance;
                }

                return 1; // userdata already on top
            });
        }

        static void methods(Machine* vm) {
            // ------------------------------------------------------------------
            // thread:resume()
            // ------------------------------------------------------------------
            vm_module::bind_method<Instance>(vm, base_name, "resume", [](auto vm, auto self, auto& id) -> int {
                if (!self || self->destroyed || !self->thread_vm) { vm->push_value(false); return 1; }

                auto instance = fetch_instance(self->id);
                if (!instance) { vm->push_value(false); return 1; }

                // Move exec function then self-userdata onto thread_vm.
                // Both are in the owner vm's registry — xmove is safe because
                // thread_vm is a coroutine of the same root state as owner vm.
                self->vm->get_reference(self->reference(),      true);
                self->vm->move(self->thread_vm, 1);

                self->vm->get_reference(self->self_reference(), true);
                self->vm->move(self->thread_vm, 1);

                safe_resume(instance, 1); // exec(self) — 1 arg, function not counted
                vm->push_value(true);
                return 1;
            });

            // ------------------------------------------------------------------
            // thread:destroy()
            // ------------------------------------------------------------------
            vm_module::bind_method<Instance>(vm, base_name, "destroy", [](auto vm, auto self, auto& id) -> int {
                if (!self || self->destroyed) { vm->push_value(false); return 1; }
                auto instance = fetch_instance(self->id);
                if (instance) Vital::Engine::Core::get_singleton()->push_deferred(
                    [instance]() { clean_instance(instance); });
                vm_module::release_userdata(vm, 1);
                vm->push_value(true);
                return 1;
            });

            // ------------------------------------------------------------------
            // thread:is_instance()
            // ------------------------------------------------------------------
            vm_module::bind_method<Instance>(vm, base_name, "is_instance", [](auto vm, auto self, auto& id) -> int {
                vm->push_value(self && !self->destroyed);
                return 1;
            });

            // ------------------------------------------------------------------
            // thread:get_thread() -> bool
            // Returns true when called from inside this thread's coroutine.
            // ------------------------------------------------------------------
            vm_module::bind_method<Instance>(vm, base_name, "get_thread", [](auto vm, auto self, auto& id) -> int {
                if (!self || self->destroyed || !self->thread_vm) { vm->push_value(false); return 1; }
                vm->push_value(self->thread_vm->get_state() == vm->get_state());
                return 1;
            });

            // ------------------------------------------------------------------
            // thread:pause()
            //
            // lua_yieldk is used instead of lua_yield so that the yield is safe
            // when called from inside execute()'s try/catch frame.
            // lua_yield uses longjmp which skips C++ destructors; lua_yieldk uses
            // a continuation and is fully safe through C++ frames in Lua 5.4.
            // The continuation returns 0 values — pause() has no return value.
            // ------------------------------------------------------------------
            vm_module::bind_method<Instance>(vm, base_name, "pause", [](auto vm, auto self, auto& id) -> int {
                if (!self || self->destroyed || !vm->is_virtual()) { vm->push_value(false); return 1; }
                return lua_yieldk(vm->get_state(), 0, 0,
                    [](lua_State*, int, lua_KContext) -> int { return 0; });
            });

            // ------------------------------------------------------------------
            // thread:sleep(duration)
            //
            // Same lua_yieldk rationale as pause(). The timer fires on the main
            // thread via push_deferred and calls safe_resume(instance, 0).
            // ------------------------------------------------------------------
            vm_module::bind_method<Instance>(vm, base_name, "sleep", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(duration)")
                    .require(2, &Machine::is_number)
                    .validate(2, [](auto vm, int index) { return vm->get_int(index) >= 0; }, "expected >= 0");

                if (!self || self->destroyed || self->sleeping) { vm->push_value(false); return 1; }

                int duration    = vm->get_int(2);
                self->sleeping  = true;
                auto instance   = fetch_instance(self->id);
                auto weak       = std::weak_ptr<Instance>(instance);

                Tool::Timer::create([weak](Tool::Timer*, int) {
                    Vital::Engine::Core::get_singleton()->push_deferred([weak]() {
                        auto instance = weak.lock();
                        if (!instance || instance->destroyed) return;
                        instance->sleeping = false;
                        safe_resume(instance, 0);
                    });
                }, duration, 1);

                return lua_yieldk(vm->get_state(), 0, 0,
                    [](lua_State*, int, lua_KContext) -> int { return 0; });
            });

            // ------------------------------------------------------------------
            // thread:await(promise) -> resolved_bool, value1, value2, ...
            //
            // If the promise is already settled, values are pushed immediately
            // from the serialised copy on the Instance (no lua_State* access).
            // If pending, the thread ID is registered and the coroutine yields.
            // The promise dispatcher resumes with (bool, v1, ...) pre-pushed.
            // The lua_yieldk continuation just returns lua_gettop() so Lua sees
            // all values that were pushed before the resume.
            // ------------------------------------------------------------------
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

                // Already settled — push from serialised values, no lua_State* required.
                if (promise_inst->state != Promise::State::Pending) {
                    lua_State* dst = vm->get_state();
                    lua_pushboolean(dst, promise_inst->resolved ? 1 : 0);
                    for (auto& v : promise_inst->serial_values) v.push(dst);
                    return 1 + static_cast<int>(promise_inst->serial_values.size());
                }

                // Pending — register thread ID and yield.
                self->awaiting = true;
                promise_inst->waiting.push_back({ self->id });

                // Record the stack depth at the point of yield. When the dispatcher
                // resumes this coroutine it pushes (bool, v1, ...) on top of the
                // existing call frame args (self at 1, promise at 2). The continuation
                // must return only the NEW values — lua_gettop(L) - base gives that count.
                // Pack base and thread_id into ctx: upper 16 bits = base, lower 16 = id.
                int base = lua_gettop(vm->get_state());
                lua_KContext ctx = (static_cast<lua_KContext>(base) << 16)
                                 | static_cast<lua_KContext>(self->id & 0xFFFF);
                return lua_yieldk(vm->get_state(), 0, ctx,
                    [](lua_State* L, int, lua_KContext ctx) -> int {
                        int base      = static_cast<int>((ctx >> 16) & 0xFFFF);
                        int thread_id = static_cast<int>(ctx        & 0xFFFF);
                        auto inst = Thread::fetch_instance(thread_id);
                        if (inst) inst->awaiting = false;
                        // Return only values pushed above the original call frame.
                        int n = lua_gettop(L) - base;
                        return n > 0 ? n : 0;
                    }
                );
            });
        }

        // ------------------------------------------------------------------
        // Thread::clean  — called by Machine::clear_environment_id
        //
        // Two-pass: mark ALL instances destroyed before deleting any of them.
        // This prevents a sleeping thread's deferred timer callback from
        // calling safe_resume on a sibling that hasn't been marked yet.
        // ------------------------------------------------------------------
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