/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: database_query.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Database Query APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if !defined(VSDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/API/utility/promise.h>


/////////////////////////////////
// Vital: API: Database_Query //
/////////////////////////////////

namespace Vital::Sandbox::API {
    struct Database_Query : vm_module {
        inline static const std::string base_name = "db_query";
        using base_class = Tool::Database::Query;

        struct Instance : vm_instance<Instance> {
            using Owner = Database_Query;
            base_class* query = nullptr;

            bool is_alive() const { 
                return query ? true : false; 
            }

            void clean() {
                auto instance = shared_from_this();
                if (!instance -> erase()) return;
                if (instance -> query) {
                    instance -> query -> destroy();
                    instance -> query = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static std::unordered_map<std::string, std::string> read_table(Machine* vm, int index) {
            std::unordered_map<std::string, std::string> result;
            auto state = vm -> get_state();
            vm -> push_nil();
            while (lua_next(state, index)) {
                if (vm -> is_string(-2)) {
                    std::string key = vm -> get_string(-2);
                    std::string value;
                    if (vm -> is_string(-1) || vm -> is_number(-1)) value = vm -> get_string(-1);
                    result[key] = value;
                }
                vm -> pop(1);
            }
            return result;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Database_Query>(vm, base_name);
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, base_name, "select", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(...)")
                    .require(2, &Machine::is_string);

                int count = vm -> get_count();
                for (int i = 2; i <= count; i++) {
                    if (!vm -> is_string(i)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("\n> Reason: invalid column name #{}", i - 1));
                    self -> query -> select.push_back(vm -> get_string(i));
                }
                self -> get_ref(self -> self_reference());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "where", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(column, op, value)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string);

                auto column = vm -> get_string(2);
                auto op = vm -> get_string(3);
                auto value = vm -> get_string(4);
                self -> query -> where.emplace_back(column, op, value);
                self -> get_ref(self -> self_reference());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "insert", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(data)")
                    .require(2, &Machine::is_table);

                self -> query -> data = read_table(vm, 2);
                self -> query -> query_type = "insert";
                self -> get_ref(self -> self_reference());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "delete", [](auto vm, auto self, auto& id) -> int {
                self -> query -> query_type = "delete";
                self -> get_ref(self -> self_reference());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "update", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(data)")
                    .require(2, &Machine::is_table);

                self -> query -> data = read_table(vm, 2);
                self -> query -> query_type = "update";
                self -> get_ref(self -> self_reference());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "fetch", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(limit = 0)")
                    .optional(2, &Machine::is_number);

                if (vm -> is_number(2)) self -> query -> limit = vm -> get_int(2);
                auto instance_id = self -> id;
                auto promise_id = Promise::make(vm) -> id;

                Tool::Thread::create([promise_id, instance_id](Tool::Thread*) {
                    auto instance = Instance::find(instance_id);
                    auto promise = Promise::Instance::find(promise_id);
                    if (!promise) { instance -> clean(); return; }
                    if (!instance) { Promise::settle(promise, Promise::State::Rejected, promise -> vm, 0, 0); return; }
                    
                    auto vm = promise -> vm;
                    try {
                        auto rows = instance -> query -> db -> fetch(instance -> query);
                        instance -> clean();
                        int index = 1;
                        vm -> create_table();
                        for (const auto& row : rows) {
                            vm -> create_table();
                            for (const auto& [column, cell] : row) {
                                vm -> table_set_value(column, cell);
                            }
                            vm -> set_table_field(index++, -2);
                        }
                        Promise::settle(promise, Promise::State::Resolved, vm, vm -> get_count(), 1);
                        vm -> pop(1);
                    }
                    catch (const std::runtime_error& error) {
                        instance -> clean();
                        vm -> push_value(std::string(error.what()));
                        Promise::settle(promise, Promise::State::Rejected, vm, vm -> get_count(), 1);
                        vm -> pop(1);
                    }
                }) -> detach();
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "alter", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(actions)")
                    .require(2, &Machine::is_table);

                int index = 2;
                Tool::Database::SchemaActions actions;
                vm -> get_table_field("add", index);
                if (vm -> is_table(-1)) {
                    auto steps = Database::read_schema_actions(vm, vm -> get_count(), Tool::Database::SchemaAction::Type::Add);
                    actions.insert(actions.end(), steps.begin(), steps.end());
                }
                vm -> pop(1);
                vm -> get_table_field("modify", index);
                if (vm -> is_table(-1)) {
                    auto steps = Database::read_schema_actions(vm, vm -> get_count(), Tool::Database::SchemaAction::Type::Modify);
                    actions.insert(actions.end(), steps.begin(), steps.end());
                }
                vm -> pop(1);
                vm -> get_table_field("drop", index);
                if (vm -> is_table(-1)) {
                    auto step_index = vm -> get_count();
                    for (int i = 1; i <= vm -> get_length(step_index); i++) {
                        vm -> get_table_field(i, step_index);
                        if (vm -> is_string(-1)) {
                            Tool::Database::SchemaAction step;
                            step.type   = Tool::Database::SchemaAction::Type::Drop;
                            step.column = vm -> get_string(-1);
                            actions.push_back(step);
                        }
                        vm -> pop(1);
                    }
                }
                vm -> pop(1);
                if (actions.empty()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "\n> Reason: no actions specified");

                auto db = self -> query -> db;
                auto table = self -> query -> table;
                auto promise_id = Promise::make(vm) -> id;
                self -> clean();

                Tool::Thread::create([promise_id, db, table, actions](Tool::Thread*) {
                    auto promise = Promise::Instance::find(promise_id);
                    if (!promise) return;

                    auto vm = promise -> vm;
                    try {
                        db -> alter(table, actions);
                        vm -> push_value(true);
                        Promise::settle(promise, Promise::State::Resolved, vm, vm -> get_count(), 1);
                        vm -> pop(1);
                    }
                    catch (const std::runtime_error& error) {
                        vm -> push_value(std::string(error.what()));
                        Promise::settle(promise, Promise::State::Rejected, vm, vm -> get_count(), 1);
                        vm -> pop(1);
                    }
                }) -> detach();
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "drop", [](auto vm, auto self, auto& id) -> int {
                auto db = self -> query -> db;
                auto table = self -> query -> table;
                auto promise_id = Promise::make(vm) -> id;
                self -> clean();

                Tool::Thread::create([promise_id, db, table](Tool::Thread*) {
                    auto promise = Promise::Instance::find(promise_id);
                    if (!promise) return;

                    auto vm = promise -> vm;
                    try {
                        db -> drop(table);
                        vm -> push_value(true);
                        Promise::settle(promise, Promise::State::Resolved, vm, vm -> get_count(), 1);
                        vm -> pop(1);
                    }
                    catch (const std::runtime_error& error) {
                        vm -> push_value(std::string(error.what()));
                        Promise::settle(promise, Promise::State::Rejected, vm, vm -> get_count(), 1);
                        vm -> pop(1);
                    }
                }) -> detach();
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "truncate", [](auto vm, auto self, auto& id) -> int {
                auto db = self -> query -> db;
                auto table = self -> query -> table;
                auto promise_id = Promise::make(vm) -> id;
                self -> clean();

                Tool::Thread::create([promise_id, db, table](Tool::Thread*) {
                    auto promise = Promise::Instance::find(promise_id);
                    if (!promise) return;

                    auto vm = promise -> vm;
                    try {
                        db -> truncate(table);
                        vm -> push_value(true);
                        Promise::settle(promise, Promise::State::Resolved, vm, vm -> get_count(), 1);
                        vm -> pop(1);
                    }
                    catch (const std::runtime_error& error) {
                        vm -> push_value(std::string(error.what()));
                        Promise::settle(promise, Promise::State::Rejected, vm, vm -> get_count(), 1);
                        vm -> pop(1);
                    }
                }) -> detach();
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "execute", [](auto vm, auto self, auto& id) -> int {
                auto instance_id = self -> id;
                auto promise_id = Promise::make(vm) -> id;

                Tool::Thread::create([promise_id, instance_id](Tool::Thread*) {
                    auto instance = Instance::find(instance_id);
                    auto promise = Promise::Instance::find(promise_id);
                    if (!promise) { instance -> clean(); return; }
                    if (!instance) { Promise::settle(promise, Promise::State::Rejected, promise -> vm, 0, 0); return; }

                    auto vm = promise -> vm;
                    try {
                        bool result = instance -> query -> db -> execute(instance -> query);
                        instance -> clean();
                        vm -> push_value(result);
                        Promise::settle(promise, Promise::State::Resolved, vm, vm -> get_count(), 1);
                        vm -> pop(1);
                    }
                    catch (const std::runtime_error& error) {
                        instance -> clean();
                        vm -> push_value(std::string(error.what()));
                        Promise::settle(promise, Promise::State::Rejected, vm, vm -> get_count(), 1);
                        vm -> pop(1);
                    }
                }) -> detach();
                return 1;
            });
        }

        static void clean(const std::string& env) {
            Instance::collect_env(env);
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Database_Query : vm_module {};
}
#endif