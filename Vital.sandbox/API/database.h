/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: database.h
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


///////////////////////////
// Vital: API: Database //
///////////////////////////

namespace Vital::Sandbox::API {
    struct DatabaseQuery : vm_module {
        inline static const std::string base_name = "db_query";
        using base_class = Tool::Database::QueryBuilder;

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
            vm_module::register_type<DatabaseQuery>(vm, base_name);
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<base_class>(vm, base_name, "destroy", [](auto vm, auto self, auto& id) -> int {
                self -> destroy();
                vm_module::release_userdata(vm, 1);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "drop", [](auto vm, auto self, auto& id) -> int {
                self -> db -> drop(self -> table);
                self -> destroy();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "truncate", [](auto vm, auto self, auto& id) -> int {
                self -> db -> truncate(self -> table);
                self -> destroy();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "alter", [](auto vm, auto self, auto& id) -> int {
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
                            step.type = Tool::Database::SchemaAction::Type::Drop;
                            step.column = vm -> get_string(-1);
                            actions.push_back(step);
                        }
                        vm -> pop(1);
                    }
                }
                vm -> pop(1);
                if (actions.empty()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, "\n> Reason: no actions specified");
                self -> db -> alter(self -> table, actions);
                self -> destroy();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "fetch", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(limit = 0)")
                    .optional(2, &Machine::is_number);

                if (vm -> is_number(2)) self -> limit = vm -> get_int(2);
                int index = 1;
                auto rows = self -> db -> fetch(self);
                vm -> create_table();
                for (const auto& row : rows) {
                    vm -> create_table();
                    for (const auto& [column, cell] : row) {
                        vm -> table_set_value(column, cell);
                    }
                    vm -> set_table_field(index++, -2);
                }
                self -> destroy();
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "execute", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> db -> execute(self));
                self -> destroy();
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "select", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(...)")
                    .require(2, &Machine::is_string);

                int count = vm -> get_count();
                for (int i = 2; i <= count; i++) {
                    if (!vm -> is_string(i)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, "\n> Reason: invalid column name");
                    self -> select.push_back(vm -> get_string(i));
                }
                vm -> create_object(base_name, self);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "where", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(column, op, value)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_string);

                auto column = vm -> get_string(2);
                auto op = vm -> get_string(3);
                auto value = vm -> get_string(4);
                self -> where.emplace_back(column, op, value);
                vm -> create_object(base_name, self);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "insert", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(data)")
                    .require(2, &Machine::is_table);

                self -> data = read_table(vm, 2);
                self -> query_type = "insert";
                vm -> create_object(base_name, self);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "delete", [](auto vm, auto self, auto& id) -> int {
                self -> query_type = "delete";
                vm -> create_object(base_name, self);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "update", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(data)")
                    .require(2, &Machine::is_table);

                self -> data = read_table(vm, 2);
                self -> query_type = "update";
                vm -> create_object(base_name, self);
                return 1;
            });
        }
    };

    struct Database : vm_module {
        inline static const std::string base_name = "database";
        using base_class = Tool::Database;

        static base_class::Column read_schema_definition(Machine* vm, int index) {
            base_class::Column definition;
            vm -> get_table_field("type", index); definition.type = vm -> is_string(-1) ? vm -> get_string(-1) : definition.type;
            vm -> get_table_field("primary", index); definition.primary = vm -> is_bool(-1) ? vm -> get_bool(-1) : definition.primary;
            vm -> get_table_field("autoincrement", index); definition.autoincrement = vm -> is_bool(-1) ? vm -> get_bool(-1) : definition.autoincrement;
            vm -> get_table_field("nullable", index); definition.nullable = vm -> is_bool(-1) ? vm -> get_bool(-1) : definition.nullable;
            vm -> pop(4);
            return definition;
        }

        static base_class::SchemaActions read_schema_actions(Machine* vm, int index, base_class::SchemaAction::Type action) {
            base_class::SchemaActions actions;
            auto state = vm -> get_state();
            vm -> push_nil();
            while (lua_next(state, index)) {
                if (!vm -> is_string(-2) || !vm -> is_table(-1)) {
                    vm -> pop(1);
                    continue;
                }
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
                auto object = base_class::create(host, user, password, database, port);
                vm -> create_object(base_name, object);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<base_class>(vm, base_name, "destroy", [](auto vm, auto self, auto& id) -> int {
                self -> destroy();
                vm_module::release_userdata(vm, 1);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_connected", [](auto vm, auto self, auto& id) -> int {
                vm -> push_value(self -> is_connected());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "define", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(table, schema)")
                    .require(2, &Machine::is_string)
                    .require(3, &Machine::is_table);

                auto state = vm -> get_state();
                auto table = vm -> get_string(2);
                base_class::TableSchema columns;
                vm -> push_nil();
                while (lua_next(state, 3)) {
                    if (!vm -> is_string(-2) || !vm -> is_table(-1)) {
                        vm -> pop(1);
                        continue;
                    }
                    auto column = vm -> get_string(-2);
                    columns[column] = read_schema_definition(vm, vm -> get_count());
                    vm -> pop(1);
                }
                self -> define(table, columns);
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "sync", [](auto vm, auto self, auto& id) -> int {
                self -> sync();
                vm -> push_value(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "table", [](auto vm, auto self, auto& id) -> int {
                vm_args(vm, id, "(name)")
                    .require(2, &Machine::is_string);

                auto name = vm -> get_string(2);
                auto query = self -> table(name);
                vm -> create_object(DatabaseQuery::base_name, query);
                return 1;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct DatabaseQuery : vm_module {};
    struct Database : vm_module {};
}
#endif