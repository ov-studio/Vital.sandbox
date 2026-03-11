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

        static std::unordered_map<std::string, std::string> read_table(lua_State* state, int index) {
            std::unordered_map<std::string, std::string> result;
            lua_pushnil(state);
            while (lua_next(state, index)) {
                if (lua_isstring(state, -2)) {
                    std::string key = lua_tostring(state, -2);
                    std::string val;
                    if (lua_isstring(state, -1) || lua_isnumber(state, -1)) val = lua_tostring(state, -1);
                    result[key] = val;
                }
                lua_pop(state, 1);
            }
            return result;
        }

        static void bind(Machine* vm) {
            vm_module::register_type<DatabaseQuery>(vm, base_name);
        }

        static void methods(Machine* vm) {
            vm_module::bind_method<base_class>(vm, base_name, "drop", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                self -> db -> drop(vm -> get_string(2));
                vm -> push_bool(true);
                return 1;
            });
    
            vm_module::bind_method<base_class>(vm, base_name, "truncate", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                self -> db -> truncate(vm -> get_string(2));
                vm -> push_bool(true);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "fetch", [](auto vm, auto self) -> int {
                int index = 1;
                auto rows = self -> db -> fetch(self);
                vm -> create_table();
                for (const auto& row : rows) {
                    vm -> create_table();
                    for (const auto& [col, val] : row) {
                        vm -> push_string(val);
                        vm -> set_table_field(col, -2);
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
                auto col = vm -> get_string(2);
                auto op = vm -> get_string(3);
                auto val = vm -> get_string(4);
                self -> wheres.emplace_back(col, op, val);
                vm -> create_object(DatabaseQuery::base_name, self);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "insert", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_table(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                self -> data = read_table(vm -> get_state(), 2);
                self -> query_type = "insert";
                vm -> create_object(base_name, self);
            });

            vm_module::bind_method<base_class>(vm, base_name, "delete", [](auto vm, auto self) -> int {
                self -> query_type = "delete";
                vm -> create_object(base_name, self);
                return 1;
            });

            vm_module::bind_method<base_class>(vm, base_name, "update", [](auto vm, auto self) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_table(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                self -> data = read_table(vm -> get_state(), 2);
                self -> query_type = "update";
                vm -> create_object(base_name, self);
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
                auto db_name = vm -> get_string(4);
                auto port = vm -> is_number(5) ? static_cast<unsigned int>(vm -> get_int(5)) : 3306u;
                auto* object = Vital::Tool::Database::create(host, user, password, db_name, port);
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
                Vital::Tool::Database::TableSchema columns;
                auto* state = vm -> get_state();
                lua_pushnil(state);
                while (lua_next(state, 3)) {
                    if (!lua_isstring(state, -2) || !lua_istable(state, -1)) {
                        lua_pop(state, 1);
                        continue;
                    }
                    auto column = std::string(lua_tostring(state, -2));
                    int col_idx = lua_gettop(state);
                    Vital::Tool::Database::Column def;
                    lua_getfield(state, col_idx, "type");
                    def.type = lua_isstring(state, -1) ? std::string(lua_tostring(state, -1)) : "VARCHAR(255)";
                    lua_pop(state, 1);
                    lua_getfield(state, col_idx, "primary");
                    def.primary = lua_isboolean(state, -1) ? lua_toboolean(state, -1) : false;
                    lua_pop(state, 1);
                    lua_getfield(state, col_idx, "autoincrement");
                    def.autoincrement = lua_isboolean(state, -1) ? lua_toboolean(state, -1) : false;
                    lua_pop(state, 1);
                    lua_getfield(state, col_idx, "nullable");
                    def.nullable = lua_isboolean(state, -1) ? lua_toboolean(state, -1) : true;
                    lua_pop(state, 1);
                    columns[column] = def;
                    lua_pop(state, 1);
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