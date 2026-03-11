/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: api: database.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Database APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/machine.h>


////////////////////////////////////
// Vital: Sandbox: API: Database //
////////////////////////////////////

namespace Vital::Sandbox::API {
    struct DatabaseQuery : vm_module {
        inline static const std::string base_name = "database_query";
        using base_class = Vital::Tool::Database::QueryBuilder;

        static std::unordered_map<std::string, std::string> read_table(Machine* vm, int index) {
            std::unordered_map<std::string, std::string> result;
            auto* state = vm -> get_state();
            lua_pushnil(state);
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
            vm_module::bind_method<base_class>(vm, base_name, "truncate", [](auto vm, auto self) -> int {
                self -> db -> truncate(self -> table);
                self -> destroy();
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "drop", [](auto vm, auto self) -> int {
                self -> db -> drop(self -> table);
                self -> destroy();
                vm -> push_bool(true);
                return 1;
            });
            
            vm_module::bind_method<base_class>(vm, base_name, "fetch", [](auto vm, auto self) -> int {
                int index = 1;
                auto rows = self -> db -> fetch(self);
                vm -> create_table();
                for (const auto& row : rows) {
                    vm -> create_table();
                    for (const auto& [column, value] : row) {
                        vm -> push_string(value);
                        vm -> set_table_field(column, -2);
                    }
                    vm -> set_table_field(index++, -2);
                }
                self -> destroy();
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "execute", [](auto vm, auto self) -> int {
                vm -> push_bool(self -> db -> execute(self));
                self -> destroy();
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "select", [](auto vm, auto self) -> int {
                int count = vm -> get_arg_count();
                if (count < 2) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                for (int i = 2; i <= count; i++) {
                    if (!vm -> is_string(i)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                    self -> select_cols.push_back(vm -> get_string(i));
                }
                vm -> create_object(base_name, self);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "where", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 4) || (!vm -> is_string(2)) || (!vm -> is_string(3))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto column = vm -> get_string(2);
                auto op = vm -> get_string(3);
                auto value = vm -> get_string(4);
                self -> wheres.emplace_back(column, op, value);
                vm -> create_object(base_name, self);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "insert", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_table(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                self -> data = read_table(vm, 2);
                self -> query_type = "insert";
                vm -> create_object(base_name, self);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "delete", [](auto vm, auto self) -> int {
                self -> query_type = "delete";
                vm -> create_object(base_name, self);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "update", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_table(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                self -> data = read_table(vm, 2);
                self -> query_type = "update";
                vm -> create_object(base_name, self);
                return 1;
            });
        }
    };

    struct Database : vm_module {
        inline static const std::string base_name = "database";
        using base_class = Vital::Tool::Database;

        static void bind(Machine* vm) {
            vm_module::register_type<Database>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 4) || (!vm -> is_string(1)) || (!vm -> is_string(2)) || (!vm -> is_string(3)) || (!vm -> is_string(4))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto host = vm -> get_string(1);
                auto user = vm -> get_string(2);
                auto password = vm -> get_string(3);
                auto database = vm -> get_string(4);
                auto port = vm -> is_number(5) ? static_cast<unsigned int>(vm -> get_int(5)) : 3306u;
                auto* object = Vital::Tool::Database::create(host, user, password, database, port);
                vm -> create_object(base_name, object);
                return 1;
            });
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<base_class>(vm, base_name, "destroy", [](auto vm, auto self) -> int {
                self -> destroy();
                vm_module::release_userdata(vm, 1);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_connected", [](auto vm, auto self) -> int {
                vm -> push_bool(self -> is_connected());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "define", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 3) || (!vm -> is_string(2)) || (!vm -> is_table(3))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto table = vm -> get_string(2);
                auto* state = vm -> get_state();
                Vital::Tool::Database::TableSchema columns;
                auto* state = vm -> get_state();
                vm -> push_nil();
                while (lua_next(state, 3)) {
                    if (!vm -> is_string(-2) || !vm -> is_table(-1)) {
                        vm -> pop(1);
                        continue;
                    }
                    auto column = vm -> get_string(-2);
                    int index = vm -> get_arg_count();
                    Vital::Tool::Database::Column definition;
                    vm -> get_table_field("type", index);
                    definition.type = vm -> is_string(-1) ? vm -> get_string(-1) : "VARCHAR(255)";
                    vm -> pop(1);
                    vm -> get_table_field("primary", index);
                    definition.primary = vm -> is_bool(-1) ? vm -> get_bool(-1) : false;
                    vm -> pop(1);
                    vm -> get_table_field("autoincrement", index);
                    definition.autoincrement = vm -> is_bool(-1) ? vm -> get_bool(-1) : false;
                    vm -> pop(1);
                    vm -> get_table_field("nullable", index);
                    definition.nullable = vm -> is_bool(-1) ? vm -> get_bool(-1) : true;
                    vm -> pop(1);
                    columns[column] = definition;
                    vm -> pop(1);
                }
                self -> define(table, columns);
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "sync", [](auto vm, auto self) -> int {
                self -> sync();
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "table", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto name = vm -> get_string(2);
                auto* query = self -> table(name);
                vm -> create_object(DatabaseQuery::base_name, query);
                return 1;
            });
        }
    };
}