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
            // vm_owned: true  = this Instance is responsible for deleting thread_vm.
            // safe_resume atomically sets it false BEFORE calling Machine::resume(),
            // so if Machine::resume() calls `delete this` on the coroutine Machine,
            // no other path will also attempt to delete it.
            // After a yield, safe_resume restores it to true.
            // Any code path that wants to access thread_vm MUST hold vm_owned == true
            // (checked via vm_owned.load()) or own the exchange result.
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

        // ------------------------------------------------------------------
        // clean_instance
        //
        // Removes the instance from the buffer and releases all resources.
        //
        // thread_vm ownership rule:
        //   vm_owned == true  → we delete thread_vm here.
        //   vm_owned == false → safe_resume transferred ownership to
        //                       Machine::resume() which called `delete this`.
        //                       We must NOT delete again.
        //
        // We null thread_vm BEFORE deleting so that any concurrent path
        // that still holds the Instance shared_ptr cannot reach the
        // dangling Machine* during the delete.
        // ------------------------------------------------------------------
        static void clean_instance(std::shared_ptr<Instance> instance) {
            if (!instance) return;
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (buffer.find(instance->id) == buffer.end()) return;
                buffer.erase(instance->id);
            }
            instance->destroyed = true;

            // Atomically claim ownership. True → we must delete. False → already freed.
            if (instance->vm_owned.exchange(false)) {
                // Null the pointer FIRST so any concurrent reader sees nullptr
                // and bails before we free the backing memory.
                Machine* tvm = instance->thread_vm;
                instance->thread_vm = nullptr;
                if (tvm) delete tvm;
            } else {
                // Machine::resume() already called `delete this` on thread_vm.
                // Just null our pointer — the memory is gone.
                instance->thread_vm = nullptr;
            }

            if (instance->vm) {
                vm_module::release_userdata_ptr(instance->userdata);
                instance->vm->del_reference(instance->self_reference());
                instance->vm->del_reference(instance->reference());
                instance->vm = nullptr;
            }
        }

        // ------------------------------------------------------------------
        // safe_resume — the ONLY place that calls Machine::resume().
        //
        // Key invariant: vm_owned must be true on entry. We atomically
        // set it false before calling resume() so no concurrent path can
        // also delete thread_vm. If the coroutine yields we restore it to
        // true; if it finishes (Machine::resume → delete this) we leave it
        // false and null thread_vm ourselves before cleaning up.
        //
        // FIX 1: Pre-resume lua_status check.
        // We verify the coroutine is actually resumable (LUA_OK = not yet
        // started, LUA_YIELD = suspended) before calling lua_resume. Calling
        // lua_resume on a dead coroutine makes Lua call luaS_new to intern
        // "cannot resume dead coroutine", which walks the main state's string
        // table through an internal pointer at offset 0x18. If that state is
        // partially freed the result is an access violation in luaS_new or
        // unroll().
        //
        // FIX 2: No post-resume Lua API calls on raw_state beyond lua_status.
        // Machine::resume() now logs errors before `delete this` while the
        // state is still fully alive. Any call to lua_tostring on raw_state
        // after Machine::resume() returns is a potential UAF because `delete
        // this` inside resume() frees the Machine object.
        //
        // FIX 3: Machine map membership check in the promise dispatcher.
        // Before pushing values onto dst (the raw coroutine lua_State), we
        // verify the state is still registered in the machines map via
        // Machine::fetch_machine(dst). If it has already been erased by
        // clean_instance / ~Machine, dst points to recycled memory and any
        // lua_push* call would corrupt the heap.
        // ------------------------------------------------------------------
        static bool safe_resume(std::shared_ptr<Instance> instance, int args) {
            if (!instance || instance->destroyed) return false;
            // Only proceed if we hold ownership. This also prevents a second
            // concurrent safe_resume from racing on the same instance.
            if (!instance->vm_owned.load()) return false;
            if (!instance->thread_vm)       return false;

            if (!instance->vm) {
                // Owner vm already torn down — we still own thread_vm, delete it.
                Machine* tvm = instance->thread_vm;
                instance->thread_vm = nullptr;
                instance->vm_owned.store(false);
                if (tvm) delete tvm;
                clean_instance(instance);
                return false;
            }

            // Capture the raw Lua state BEFORE we transfer ownership.
            // After resume() returns the Machine* may have been freed by
            // `delete this` inside Machine::resume() — never dereference
            // instance->thread_vm after the resume() call.
            vm_state* raw_state = instance->thread_vm->get_state();
            if (!raw_state) return false;

            // FIX 1: Guard against resuming a dead or errored coroutine.
            // LUA_OK    → coroutine not yet started (first resume).
            // LUA_YIELD → coroutine suspended mid-execution (normal resume).
            // Anything else → coroutine finished or errored; resuming it
            // causes Lua to intern an error string via luaS_new which reads
            // a pointer at offset 0x18 inside the (potentially freed) state,
            // producing the access violation seen in unroll() / luaS_new.
            {
                int pre_status = lua_status(raw_state);
                if (pre_status != LUA_OK && pre_status != LUA_YIELD) {
                    instance->thread_vm = nullptr;
                    instance->vm_owned.store(false);
                    clean_instance(instance);
                    return false;
                }
            }

            // Transfer ownership atomically. From this point, thread_vm's
            // Machine lifetime is controlled by Machine::resume().
            instance->vm_owned.store(false);

            instance->thread_vm->resume(args);
            // thread_vm MAY be freed now (delete this in Machine::resume).
            // Machine::resume() has already logged any error before deleting.
            // Do NOT call lua_tostring or any other Lua API here — UAF risk.

            // FIX 2: lua_status is the only Lua API call we allow on raw_state
            // post-resume. It reads a single int field from the lua_State struct
            // and is safe even if the Machine wrapper has been freed, as long as
            // the lua_State memory itself is still alive (it is — only the parent
            // main state's lua_close frees coroutine thread memory, and the parent
            // is still running). We use this solely to decide vm_owned.
            int status = lua_status(raw_state);
            if (status != LUA_YIELD) {
                // Machine::resume() already freed thread_vm via `delete this`.
                // Null our pointer before clean_instance so it doesn't try to
                // free it a second time (vm_owned is false, but belt-and-suspenders).
                instance->thread_vm = nullptr;
                clean_instance(instance);
                return false;
            }

            // Coroutine is still alive — reclaim ownership so future resumes
            // (sleep timer, promise dispatcher) can proceed.
            instance->vm_owned.store(true);
            return true;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Thread>(vm, base_name);

            // ------------------------------------------------------------------
            // Promise → thread resume dispatcher.
            // Registered once per Machine construction (harmless to overwrite).
            // Called from flush_deferred_queue on the main thread.
            //
            // CRITICAL: Before accessing thread_vm, we must verify BOTH that the
            // instance is not destroyed AND that vm_owned is true. There is a
            // narrow window between safe_resume setting vm_owned=false and
            // setting instance->thread_vm=nullptr where the instance is still in
            // the buffer (not yet erased by clean_instance) with destroyed=false
            // but thread_vm pointing to already-freed Machine memory. Checking
            // vm_owned closes that window completely.
            //
            // FIX 3: Before pushing anything onto dst, confirm the raw lua_State
            // is still registered in Machine::machines via fetch_machine(dst).
            // Between the deferred lambda being queued and it executing on the
            // main thread, clean_instance may have run and erased the Machine
            // from the map (and called `delete tvm`). dst then points to freed
            // memory. Pushing a value onto a freed lua_State corrupts the heap
            // and produces the "attempt to call a nil value (method 'await')" /
            // garbage-string errors and the `ci was nullptr` crash in unroll()
            // on subsequent restarts.
            // ------------------------------------------------------------------
            Promise::register_resume_dispatcher(
                [](int thread_id, bool resolved,
                   const std::vector<Promise::SerialValue>& values) {

                    auto instance = Thread::fetch_instance(thread_id);
                    if (!instance || instance->destroyed) return;

                    // Guard: thread_vm is only safe to dereference while we hold
                    // ownership (vm_owned == true). If false, safe_resume is
                    // mid-flight and thread_vm may already be freed memory.
                    if (!instance->vm_owned.load()) return;
                    if (!instance->thread_vm)       return;

                    lua_State* dst = instance->thread_vm->get_state();
                    if (!dst) return;

                    // FIX 3: Confirm dst is still a live, registered state.
                    // If clean_instance already ran, the Machine has been erased
                    // from the map and dst may point to freed memory. Any
                    // lua_push* call on a freed state corrupts the heap.
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

                // lua_newthread pushes the coroutine thread object onto the owner stack.
                // Stack before: [exec_fn]
                // Stack after create_thread(): [exec_fn, thread_obj]
                Machine* thread_vm  = vm->create_thread();
                instance->thread_vm = thread_vm;

                // Store exec ref from index 1 (stack-neutral: push + luaL_ref pops).
                // Stack stays: [exec_fn, thread_obj]
                vm->set_reference(instance->reference(), 1);

                // Pop the lua_newthread thread object — held via thread_vm Machine*.
                // Stack: [exec_fn]
                vm->pop(1);

                // Create self userdata. Stack: [exec_fn, userdata]
                vm->create_object(base_name, instance.get());
                instance->userdata = vm_module::get_userdata_ptr(vm, -1);

                // Store full-userdata ref (stack-neutral). Stack: [exec_fn]
                vm->set_reference(instance->self_reference(), -1);

                {
                    std::lock_guard<std::mutex> lock(mutex);
                    buffer[instance->id] = instance;
                }

                // Return the userdata. Stack: [exec_fn, userdata]
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

                // Move exec function then self-userdata onto thread_vm.
                // xmove is safe: thread_vm is a coroutine of the same root state.
                self->vm->get_reference(self->reference(),      true);
                self->vm->move(self->thread_vm, 1);

                self->vm->get_reference(self->self_reference(), true);
                self->vm->move(self->thread_vm, 1);

                // thread_vm stack: [exec, self_userdata]
                // resume(1) = 1 argument; the function itself is not counted.
                safe_resume(instance, 1);
                vm->push_value(true);
                return 1;
            });

            // thread:destroy()
            vm_module::bind_method<Instance>(vm, base_name, "destroy", [](auto vm, auto self, auto& id) -> int {
                if (!self || self->destroyed) { vm->push_value(false); return 1; }
                auto instance = fetch_instance(self->id);
                if (instance) Vital::Engine::Core::get_singleton()->push_deferred(
                    [instance]() { clean_instance(instance); });
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
                    Vital::Engine::Core::get_singleton()->push_deferred([weak]() {
                        auto instance = weak.lock();
                        if (!instance || instance->destroyed) return;
                        instance->sleeping = false;
                        // Guard vm_owned before resuming — same race as dispatcher.
                        if (!instance->vm_owned.load() || !instance->thread_vm) return;
                        safe_resume(instance, 0);
                    });
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

                // Already settled — push from serialised values; no lua_State* needed.
                if (promise_inst->state != Promise::State::Pending) {
                    lua_State* dst = vm->get_state();
                    lua_pushboolean(dst, promise_inst->resolved ? 1 : 0);
                    for (auto& v : promise_inst->serial_values) v.push(dst);
                    return 1 + static_cast<int>(promise_inst->serial_values.size());
                }

                // Pending — register and yield.
                self->awaiting = true;
                promise_inst->waiting.push_back({ self->id });

                // Encode stack depth + thread_id into ctx so the continuation
                // returns only the values pushed by the dispatcher (not the
                // original call frame args self=1, promise=2).
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

        // ------------------------------------------------------------------
        // Thread::clean — called by Machine::clear_environment_id.
        // Two-pass: mark ALL instances destroyed before deleting any.
        // This prevents a sleeping thread's timer deferred from calling
        // safe_resume on a sibling that is mid-teardown.
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