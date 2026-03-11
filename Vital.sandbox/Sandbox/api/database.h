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
#include <Vital.sandbox/System/database.h>


////////////////////////////////////
// Vital: Sandbox: API: Database //
////////////////////////////////////

namespace Vital::Sandbox::API {
    struct DatabaseQuery : vm_module {
        inline static const std::string base_name = "database_query";
        using base_class = Vital::System::Database::QueryBuilder;

        static std::unordered_map<std::string, std::string> read_table(lua_State* L, int index) {
            std::unordered_map<std::string, std::string> result;
            lua_pushnil(L);
            while (lua_next(L, index)) {
                if (lua_isstring(L, -2)) result[lua_tostring(L, -2)] = lua_tostring(L, -1);
                lua_pop(L, 1);
            }
            return result;
        }

        static void push_rows(Machine* vm, const std::vector<std::unordered_map<std::string, std::string>>& rows) {
            vm -> create_table();
            int row_i = 1;
            for (const auto& row : rows) {
                vm -> create_table();
                for (const auto& [col, val] : row) {
                    vm -> push_string(val);
                    vm -> set_table_field(col, -2);
                }
                vm -> set_table_field(row_i++, -2);
            }
        }

        static int set_data_query(Machine* vm, base_class* self, const std::string& query_type, int table_index) {
            self -> data = read_table(vm -> get_state(), table_index);
            self -> query_type = query_type;
            vm -> create_object(base_name, self);
            return 1;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<DatabaseQuery>(vm, base_name);
        }

        static void methods(Machine* vm) {
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
                auto col = vm -> get_string(2);
                auto op  = vm -> get_string(3);
                auto val = vm -> get_string(4);
                self -> wheres.emplace_back(col, op, val);
                vm -> create_object(DatabaseQuery::base_name, self);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "insert", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_table(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                return set_data_query(vm, self, "insert", 2);
            });

            vm_module::bind_method<base_class>(vm, base_name, "update", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_table(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto* L = vm -> get_state();
                lua_pushnil(L);
                while (lua_next(L, 2)) {
                    if (lua_isstring(L, -2) && lua_isstring(L, -1))
                        self -> data[lua_tostring(L, -2)] = lua_tostring(L, -1);
                    lua_pop(L, 1);
                }
                self -> query_type = "update";
                vm -> create_object(DatabaseQuery::base_name, self);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "delete", [](auto vm, auto self) -> int {
                self -> query_type = "delete";
                vm -> create_object(DatabaseQuery::base_name, self);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "fetch", [](auto vm, auto self) -> int {
                auto rows = self -> db -> fetch(self);
                vm -> create_table();
                int row_i = 1;
                for (const auto& row : rows) {
                    vm -> create_table();
                    for (const auto& [col, val] : row) {
                        vm -> push_string(val);
                        vm -> set_table_field(col, -2);
                    }
                    vm -> set_table_field(row_i++, -2);
                }
                self -> db -> destroy_query(self);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "execute", [](auto vm, auto self) -> int {
                vm -> push_bool(self -> db -> execute(self));
                self -> db -> destroy_query(self);
                return 1;
            });
        }
    };

    struct Database : vm_module {
        inline static const std::string base_name = "database";
        using base_class = Vital::System::Database;

        static void bind(Machine* vm) {
            vm_module::register_type<Database>(vm, base_name);

            API::bind(vm, {base_name}, "create", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 4) || (!vm -> is_string(1)) || (!vm -> is_string(2)) || (!vm -> is_string(3)) || (!vm -> is_string(4))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto host = vm -> get_string(1);
                auto user = vm -> get_string(2);
                auto password = vm -> get_string(3);
                auto db_name = vm -> get_string(4);
                auto port = vm -> is_number(5) ? static_cast<unsigned int>(vm -> get_int(5)) : 3306u;
                auto* object = Vital::System::Database::create(host, user, password, db_name, port);
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

            vm_module::bind_method<base_class>(vm, base_name, "disconnect", [](auto vm, auto self) -> int {
                self -> disconnect();
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "is_connected", [](auto vm, auto self) -> int {
                vm -> push_bool(self -> is_connected());
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "define", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 3) || (!vm -> is_string(2)) || (!vm -> is_table(3))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto table_name = vm -> get_string(2);
                Vital::System::Database::TableSchema columns;
                auto* L = vm -> get_state();
                lua_pushnil(L);
                while (lua_next(L, 3)) {
                    if (!lua_isstring(L, -2) || !lua_istable(L, -1)) { lua_pop(L, 1); continue; }
                    auto col_name = std::string(lua_tostring(L, -2));
                    int col_idx = lua_gettop(L);
                    Vital::System::Database::ColumnDef def;
                    lua_getfield(L, col_idx, "type");
                    def.type = lua_isstring(L, -1) ? std::string(lua_tostring(L, -1)) : "VARCHAR(255)";
                    lua_pop(L, 1);
                    lua_getfield(L, col_idx, "primary");
                    def.primary = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);
                    lua_getfield(L, col_idx, "autoincrement");
                    def.autoincrement = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);
                    lua_getfield(L, col_idx, "nullable");
                    def.nullable = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : true;
                    lua_pop(L, 1);
                    columns[col_name] = def;
                    lua_pop(L, 1);
                }
                self -> define(table_name, columns);
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