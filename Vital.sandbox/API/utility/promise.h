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

        enum class State {
            Pending,
            Resolved,
            Rejected
        };

        struct Instance : vm_instance<Instance> {
            using Owner = Promise;
            State state { State::Pending };
            std::vector<int> waiting;
            bool resolved = false;
            int value_count = 0;

            void clean() {
                auto instance = shared_from_this();
                if (instance->state == State::Pending) {
                    auto* vm = instance->vm;
                    if (vm) settle(instance, State::Rejected, vm, 0, 0);
                }
                if (!instance -> erase()) return;
                instance -> waiting.clear();
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        using ResumeDispatcher = std::function<void(int thread_id, bool resolved, std::shared_ptr<Instance> promise)>;
        inline static ResumeDispatcher resume_dispatcher;
        static void register_resume_dispatcher(ResumeDispatcher exec) {
            resume_dispatcher = std::move(exec);
        }

        static int push_values(std::shared_ptr<Instance> instance, Machine* dst) {
            if (!Instance::find_unlocked(instance) || !instance -> vm || instance -> value_count == 0) return 0;
            if (!instance || !instance -> vm || instance -> value_count == 0) return 0;
            auto state = dst -> get_state();
            for (int i = 1; i <= instance -> value_count; ++i) {
                int ref = instance -> get_reference(instance -> value_reference(i));
                lua_rawgeti(state, LUA_REGISTRYINDEX, ref);
            }
            return instance -> value_count;
        }

        static void settle(std::shared_ptr<Instance> instance, State result_state, Machine* vm, int args_start, int args_count) {
            if (!Instance::find_unlocked(instance) || instance -> state != State::Pending || !vm) return;
            instance -> state = result_state;
            instance -> resolved = (result_state == State::Resolved);
            instance -> value_count = args_count;
            for (int i = 0; i < args_count; ++i) instance -> set_reference(instance -> value_reference(i + 1), args_start + i);
            auto waiting = instance -> waiting;
            instance -> waiting.clear();
            bool resolved_flag = instance -> resolved;
            if (!Promise::resume_dispatcher) return;
            for (int tid : waiting) Promise::resume_dispatcher(tid, resolved_flag, instance);
        }

        static std::shared_ptr<Instance> make(Machine* vm) {
            return Instance::make(vm);
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Promise>(vm);

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                Promise::make(vm);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, "is_pending", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> state == State::Pending);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "resolve", [](auto vm, auto self, auto& id) -> int {
                if (self -> state != State::Pending) vm -> push_value(false);
                else {
                    settle(self, State::Resolved, vm, 2, vm -> get_count() - 1);
                    vm -> push_value(true);
                }
                return 1;
            });

            vm_module::bind_method<Instance>(vm, "reject", [](auto vm, auto self, auto& id) -> int {
                if (self -> state != State::Pending) vm -> push_value(false);
                else {
                    settle(self, State::Rejected, vm, 2, vm -> get_count() - 1);
                    vm -> push_value(true);
                }
                return 1;
            });
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}