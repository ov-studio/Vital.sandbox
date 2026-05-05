/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: utility: promise.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Promise APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Engine/public/core.h>


//////////////////////////
// Vital: API: Promise //
//////////////////////////

namespace Vital::Sandbox::API {
    struct Promise : vm_module {
        inline static const std::string base_name = "promise";

        enum class State { Pending, Resolved, Rejected };

        struct WaitingThread {
            int thread_instance_id = -1;
        };

        struct Instance {
            int id {};
            std::string env;
            std::atomic<bool> destroyed { false };
            State state { State::Pending };
            Machine* vm = nullptr;
            void** userdata = nullptr;
            std::vector<WaitingThread> waiting;
            bool resolved = false;
            int value_count = 0;

            std::string reference() const {
                return fmt::format("{}:{}", base_name, id);
            }
            std::string value_reference(int i) const {
                return fmt::format("{}:{}:v:{}", base_name, id, i);
            }
        };

        inline static std::unordered_map<int, std::shared_ptr<Instance>> buffer;
        inline static std::atomic<int> next_id { 1 };
        inline static std::mutex mutex;

        using ResumeDispatcher = std::function<void(int thread_id, bool resolved, std::shared_ptr<Instance> promise)>;
        inline static ResumeDispatcher resume_dispatcher;
        static void register_resume_dispatcher(ResumeDispatcher fn) {
            resume_dispatcher = std::move(fn);
        }

        // Push all settled values onto dst's lua_State* directly via lua_rawgeti.
        // We cannot use instance->vm->get_reference() here because that pushes onto
        // instance->vm's state — which may differ from dst's coroutine state.
        // Instead we read the raw ref int and rawgeti onto dst's state explicitly.
        static int push_values(std::shared_ptr<Instance> instance, Machine* dst) {
            if (!instance || !instance->vm || instance->value_count == 0) return 0;
            lua_State* L = dst->get_state();
            for (int i = 1; i <= instance->value_count; ++i) {
                int ref = instance->vm->get_reference(instance->value_reference(i));
                lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
            }
            return instance->value_count;
        }

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
            if (instance->vm) {
                for (int i = 1; i <= instance->value_count; ++i)
                    instance->vm->del_reference(instance->value_reference(i));
                vm_module::release_userdata_ptr(instance->userdata);
                instance->vm->del_reference(instance->reference());
                instance->vm = nullptr;
            }
        }

        // settle() stores each value as a named registry ref on instance->vm.
        // The registry is global to the lua_State, so refs are valid for any
        // coroutine (thread_vm) sharing the same main state.
        // args_start: absolute 1-based stack index of first value on vm's stack.
        static void settle(std::shared_ptr<Instance> instance, State result_state, Machine* vm, int args_start, int args_count) {
            if (!instance || instance->destroyed || instance->state != State::Pending || !vm) return;

            instance->state       = result_state;
            instance->resolved    = (result_state == State::Resolved);
            instance->value_count = args_count;

            for (int i = 0; i < args_count; ++i)
                vm->set_reference(instance->value_reference(i + 1), args_start + i);

            auto waiting = instance->waiting;
            instance->waiting.clear();

            bool resolved_flag = instance->resolved;
            std::vector<int> thread_ids;
            thread_ids.reserve(waiting.size());
            for (auto& wt : waiting) {
                if (wt.thread_instance_id >= 0) thread_ids.push_back(wt.thread_instance_id);
            }

            if (!Promise::resume_dispatcher) return;
            for (int tid : thread_ids) Promise::resume_dispatcher(tid, resolved_flag, instance);
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Promise>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                std::string env = vm->get_environment_id();
                auto instance = std::make_shared<Instance>();
                instance->id  = next_id.fetch_add(1);
                instance->env = env;
                instance->vm  = vm;
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    buffer[instance->id] = instance;
                }
                vm->create_object(base_name, instance.get());
                instance->userdata = vm_module::get_userdata_ptr(vm, -1);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, base_name, "destroy", [](auto vm, auto self, auto& id) -> int {
                if (self->destroyed) { vm->push_value(false); return 1; }
                self->destroyed = true;
                auto instance = fetch_instance(self->id);
                if (instance) clean_instance(instance);
                vm_module::release_userdata(vm, 1);
                vm->push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "is_pending", [](auto vm, auto self, auto& id) -> int {
                vm->push_value(!self->destroyed && self->state == State::Pending);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "resolve", [](auto vm, auto self, auto& id) -> int {
                if (self->destroyed || self->state != State::Pending) { vm->push_value(false); return 1; }
                auto instance = fetch_instance(self->id);
                if (!instance) { vm->push_value(false); return 1; }
                settle(instance, State::Resolved, vm, 2, vm->get_count() - 1);
                vm->push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "reject", [](auto vm, auto self, auto& id) -> int {
                if (self->destroyed || self->state != State::Pending) { vm->push_value(false); return 1; }
                auto instance = fetch_instance(self->id);
                if (!instance) { vm->push_value(false); return 1; }
                settle(instance, State::Rejected, vm, 2, vm->get_count() - 1);
                vm->push_value(true);
                return 1;
            });
        }

        static void clean(const std::string& env) {
            std::vector<std::shared_ptr<Instance>> to_clean;
            {
                std::lock_guard<std::mutex> lock(mutex);
                for (auto& [id, instance] : buffer) {
                    if (instance->env == env) to_clean.push_back(instance);
                }
            }
            for (auto& instance : to_clean) {
                instance->destroyed = true;
                instance->waiting.clear();
            }
            for (auto& instance : to_clean) clean_instance(instance);
        }
    };
}