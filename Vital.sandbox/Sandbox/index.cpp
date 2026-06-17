/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: index.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/machine.h>
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/console.h>
#include <Vital.sandbox/Sandbox/runtime/apis.h>


/////////////////////
// Vital: Sandbox //
/////////////////////

namespace Vital::Sandbox {
    vm_apis Machine::internal_apis = API::make_apis();

    void Machine::next_tick(std::function<void()> exec) {
        Engine::Core::get_singleton() -> enqueue([exec = std::move(exec)]() {
            enqueue(std::move(exec));
        });
    }

    void Machine::bind(const std::vector<std::string>& scope, const std::string& name, vm_bind exec) {
        Engine::Core::get_singleton() -> execute([this, scope, name, exec = std::move(exec)]() mutable {
            Tool::assert_main_thread("Machine::bind");
            // TODO: deallocate heap on sandbox vm closure??
            auto heap_exec = new vm_bind(std::move(exec));
            std::string id = vm_module::scope_id(scope) + "." + name;
            auto heap_id = new std::string(std::move(id));
            set_scope(scope);
            push_userdata(heap_exec);
            push_userdata(heap_id);
            lua_pushcclosure(state, [](vm_state* state) -> int {
                auto exec = static_cast<vm_bind*>(lua_touserdata(state, lua_upvalueindex(1)));
                auto id = static_cast<std::string*>(lua_touserdata(state, lua_upvalueindex(2)));
                auto vm = Machine::fetch_machine(state);
                return vm -> execute([&]() -> int {
                    return (*exec)(vm, *id);
                });
            }, 2);
            set_table_field(name, -2);
            pop(static_cast<int>(scope.size()));
        });
    }

    namespace API {
        void log(const std::string& mode, const std::string& message) {
            Tool::print(mode, message);
        }

        void bind(Machine* vm, const std::vector<std::string>& scope, const std::string& name, vm_bind exec) {
            vm -> bind(scope, name, std::move(exec));
        }
    }
}