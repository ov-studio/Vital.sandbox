/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: core: database.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Database APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if !defined(Vital_SDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/API/utility/promise.h>
#include <Vital.sandbox/API/core/database_query.h>


///////////////////////////
// Vital: API: Database //
///////////////////////////

namespace Vital::Sandbox::API {
    struct Database : vm_module {
        inline static const std::string base_name = "database";
        using base_class = Tool::Database;

        struct Instance : vm_instance<Instance> {
            using Owner = Database;
            base_class* db = nullptr;

            bool is_alive() const { 
                return db ? true : false;
            }
            
            void clean() {
                auto instance = shared_from_this();
                if (!Instance::erase(instance)) return;
                if (instance -> db) {
                    instance -> db -> destroy();
                    instance -> db = nullptr;
                }
                instance -> release();
            }
        };
        inline static vm_registry<Instance> registry;

        static base_class::Column read_schema_definition(Machine* vm, int index) {
            base_class::Column definition;
            vm -> get_table_field("type", index); definition.type = vm -> is_string(-1) ? vm -> get_string(-1) : definition.type;
            vm -> get_table_field("primary", index); definition.primary = vm -> is_bool(-1) ? vm -> get_bool(-1) : definition.primary;
            vm -> get_table_field("autoincrement", index); definition.autoincrement = vm -> is_bool(-1)   ? vm -> get_bool(-1) : definition.autoincrement;
            vm -> get_table_field("nullable", index); definition.nullable = vm -> is_bool(-1) ? vm -> get_bool(-1) : definition.nullable;
            vm -> pop(4);
            return definition;
        }

        static base_class::SchemaActions read_schema_actions(Machine* vm, int index, base_class::SchemaAction::Type action) {
            base_class::SchemaActions actions;
            auto state = vm -> get_state();
            vm -> push_nil();
            while (lua_next(state, index)) {
                if (!vm -> is_string(-2) || !vm -> is_table(-1)) { vm -> pop(1); continue; }
                auto column = vm -> get_string(-2);
                actions.push_back({action, column, read_schema_definition(vm, vm -> get_count())});
                vm -> pop(1);
            }
            return actions;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<Database>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm, auto& id) -> int {
                vm_args(vm, id, "(host, user, password, database, port = 3306)")
                    .require(1, &Machine::is_string)
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string)
                    .require(4, &Machine::is_string)
                    .optional(5, &Machine::is_number);

                auto host = vm -> get_string(1);
                auto user = vm -> get_string(2);
                auto password = vm -> get_string(3);
                auto database = vm -> get_string(4);
                auto port = vm -> is_number(5) ? static_cast<unsigned int>(vm -> get_int(5)) : 3306u;
                auto instance = Instance::make(vm);
                instance -> db = base_class::create(host, user, password, database, port);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<Instance>(vm, base_name, "is_connected", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> db -> is_connected());
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "define", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(table, schema)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_table);

                auto state = vm -> get_state();
                auto table = vm -> get_string(2);
                base_class::TableSchema columns;
                vm -> push_nil();
                while (lua_next(state, 3)) {
                    if (!vm -> is_string(-2) || !vm -> is_table(-1)) { vm -> pop(1); continue; }
                    auto column = vm -> get_string(-2);
                    columns[column] = read_schema_definition(vm, vm -> get_count());
                    vm -> pop(1);
                }
                self -> db -> define(table, columns);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<Instance>(vm, base_name, "sync", [](auto vm, auto self, auto& id) -> int {
                auto db = self -> db;
                auto promise_id = Promise::make(vm) -> id;
                Tool::Thread::create([promise_id, db](Tool::Thread*) {
                    auto promise = Promise::Instance::find(promise_id);
                    if (!promise) return;
                    
                    auto vm = promise -> vm;
                    try {
                        db -> sync();
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

            vm_module::bind_method<Instance>(vm, base_name, "table", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name)")
                    .require(2, &Machine::is_string);

                auto name = vm -> get_string(2);
                auto instance = Database_Query::Instance::make(vm);
                instance -> query = self -> db -> table(name);
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
    struct Database : vm_module {};
}
#endif