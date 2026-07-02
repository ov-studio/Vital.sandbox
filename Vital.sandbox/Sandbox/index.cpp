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
#include <Vital.sandbox/Sandbox/runtime/apis.h>
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Manager/public/kit.h>


/////////////////////
// Vital: Sandbox //
/////////////////////

namespace Vital::Sandbox {
    vm_apis Machine::apis = API::make_apis();

    void Machine::load_modules(Machine* vm) {
        for (auto& [name, source] : Manager::Kit::fetch_modules("lua")) {
            vm -> load_string(source, name);
        }
    }

    void Machine::next_tick(std::function<void()> exec) {
        Engine::Core::get_singleton() -> enqueue([exec = std::move(exec)]() {
            enqueue(std::move(exec));
        });
    }

    void Machine::bind(const std::vector<std::string>& scope, const std::string& name, vm_bind exec) {
        Engine::Core::get_singleton() -> execute([this, scope, name, exec = std::move(exec)]() mutable {
            Tool::assert_main_thread("Machine::bind");
            scope_set(scope);
            vm_module::push_owned<vm_bind>(state, std::move(exec));
            vm_module::push_owned<std::string>(state, vm_module::scope_id(scope) + "." + name);
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
        void bind(Machine* vm, const std::vector<std::string>& scope, const std::string& name, vm_bind exec) {
            vm -> bind(scope, name, std::move(exec));
        }
    }
}