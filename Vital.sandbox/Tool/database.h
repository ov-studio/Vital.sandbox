/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: database.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Database Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#if !defined(Vital_SDK_Client)
#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <Vital.sandbox/Tool/log.h>
#include <Vital.sandbox/Tool/stack.h>
#include <soci/soci.h>
#include <soci/mysql/soci-mysql.h>


////////////////////////////
// Vital: Tool: Database //
////////////////////////////

namespace Vital::Tool {
    class Database {
        public:
            struct Column {
                std::string type = "VARCHAR(255)";
                bool primary = false;
                bool autoincrement = false;
                bool nullable = true;
            };

            struct SchemaAction {
                enum class Type { Add, Drop, Modify };
                Type type;
                std::string column;
                Column definition;
            };

            struct QueryBuilder {
                Database* db;
                std::string table;
                std::string query_type;
                std::vector<std::string> select;
                std::vector<std::tuple<std::string, std::string, std::string>> where;
                std::unordered_map<std::string, std::string> data;
                int limit = 0;
                inline static const std::unordered_set<std::string> valid_ops = {"=", "!=", ">", "<", ">=", "<="};

                void destroy() {
                    delete this;
                }

                std::pair<std::string, std::vector<std::string>> build_where() {
                    std::string clause = " WHERE ";
                    std::vector<std::string> binds;
                    for (int i = 0; i < (int)where.size(); i++) {
                        const auto& [column, op, value] = where[i];
                        if (!db -> is_column_allowed(table, column) || !valid_ops.count(op)) throw Tool::Log::fetch("invalid-argument", Tool::Log::Type::Error, fmt::format("\n> Reason: Column '{}' or operator '{}' — invalid in '{}'", column, op, table));
                        if (i > 0) clause += " AND ";
                        clause += fmt::format("`{}` {} :w{}", column, op, i);
                        binds.push_back(value);
                    }
                    return {clause, binds};
                }

                void apply_where(std::string& sql, std::vector<std::string>& binds, std::vector<std::string>& bind_names) {
                    if (where.empty()) return;
                    auto [where_clause, where_binds] = build_where();
                    sql += where_clause;
                    for (int i = 0; i < (int)where_binds.size(); i++) {
                        bind_names.push_back(fmt::format("w{}", i));
                        binds.push_back(where_binds[i]);
                    }
                }

                void apply_limit(std::string& sql) {
                    if (limit <= 0) return;
                    sql += fmt::format(" LIMIT {}", limit);
                }
            };

            using TableSchema = std::unordered_map<std::string, Column>;
            using GlobalSchema = std::unordered_map<std::string, TableSchema>;
            using SchemaActions = std::vector<SchemaAction>;
            using Row = std::vector<std::pair<std::string, Tool::StackValue>>;
            using Rows = std::vector<Row>;
        private:
            std::unique_ptr<soci::session> session;
            GlobalSchema schema;

            static bool is_safe_identifier(const std::string& name) {
                if (name.empty() || name.size() > 64) return false;
                for (char c : name) {
                    if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') return false;
                }
                return true;
            }

            bool is_table_allowed(const std::string& table) const {
                return schema.find(table) != schema.end();
            }

            bool is_column_allowed(const std::string& table, const std::string& column) const {
                auto it = schema.find(table);
                if (it == schema.end()) return false;
                return it -> second.find(column) != it -> second.end();
            }

            std::string build_schema_definition(const std::string& column, const Column& definition) {
                std::string statement = fmt::format("`{}` {}", column, definition.type);
                if (definition.autoincrement) statement += " AUTO_INCREMENT";
                if (!definition.nullable) statement += " NOT NULL";
                return statement;
            }

            void assert_session() const {
                if (!session) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, "\n> Reason: No active session");
            }

            void assert_table(const std::string& table) const {
                if (!is_table_allowed(table)) throw Tool::Log::fetch("invalid-argument", Tool::Log::Type::Error, fmt::format("\n> Reason: Table '{}' — not defined", table));
            }

            void assert_session_and_table(const std::string& table) const {
                assert_session();
                assert_table(table);
            }

            void push_bind(const std::string& table, const std::string& column, const std::string& value, int index, std::string& columns_out, std::string& placeholders_out, std::vector<std::string>& binds, std::vector<std::string>& bind_names) {
                if (!is_column_allowed(table, column)) throw Tool::Log::fetch("invalid-argument", Tool::Log::Type::Error, fmt::format("\n> Reason: Column '{}' — not defined in '{}'", column, table));
                auto pname = fmt::format("d{}", index);
                columns_out += fmt::format("`{}`", column);
                placeholders_out += fmt::format(":{}", pname);
                bind_names.push_back(pname);
                binds.push_back(value);
            }

            void run_statement(const std::string& sql, const std::vector<std::string>& binds, const std::vector<std::string>& bind_names, soci::row* row_out = nullptr) {
                soci::statement st(*session);
                st.alloc();
                st.prepare(sql);
                for (int i = 0; i < (int)binds.size(); i++) {
                    st.exchange(soci::use(binds[i], bind_names[i]));
                }
                if (row_out) st.exchange(soci::into(*row_out));
                st.define_and_bind();
                st.execute(row_out == nullptr);
            }
        public:
            Database() = default;
            ~Database() = default;

            static Database* create(const std::string& host, const std::string& user, const std::string& password, const std::string& database, unsigned int port = 3306) {
                auto db = new Database();
                std::string connection = fmt::format("host={} port={} user={} dbname={}", host, port, user, database);
                if (!password.empty()) connection += fmt::format(" password={}", password);
                db -> session = std::make_unique<soci::session>(soci::mysql, connection);
                return db;
            }

            void destroy() {
                if (session) {
                    session -> close();
                    session.reset();
                }
                delete this;
            }

            bool is_connected() const {
                return session != nullptr && session -> is_connected();
            }

            void define(const std::string& table, const TableSchema& columns) {
                if (!is_safe_identifier(table)) throw Tool::Log::fetch("invalid-argument", Tool::Log::Type::Error, fmt::format("\n> Reason: Table '{}' — invalid identifier", table));
                for (const auto& [column, definition] : columns) {
                    if (!is_safe_identifier(column)) throw Tool::Log::fetch("invalid-argument", Tool::Log::Type::Error, fmt::format("\n> Reason: Column '{}' — invalid identifier", column));
                }
                schema[table] = columns;
            }

            void sync() {
                if (!session) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error,
                    "\n> Reason: No active session");
                for (const auto& [table, columns] : schema) {
                    std::string sql = fmt::format("CREATE TABLE IF NOT EXISTS `{}` (", table);
                    std::string primary_key;
                    bool first = true;
                    for (const auto& [column, definition] : columns) {
                        if (!first) sql += ", ";
                        first = false;
                        sql += build_schema_definition(column, definition);
                        if (definition.primary) primary_key = column;
                    }
                    if (!primary_key.empty()) sql += fmt::format(", PRIMARY KEY (`{}`)", primary_key);
                    sql += ")";
                    *session << sql;
                }
            }

            QueryBuilder* table(const std::string& name) {
                assert_table(name);
                auto query = new QueryBuilder();
                query -> db = this;
                query -> table = name;
                return query;
            }

            void drop(const std::string& table) {
                if (!session) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, "\n> Reason: No active session");
                if (!is_table_allowed(table)) throw Tool::Log::fetch("invalid-argument", Tool::Log::Type::Error, fmt::format("\n> Reason: Table '{}' — not defined", table));
                *session << fmt::format("DROP TABLE IF EXISTS `{}`", table);
                schema.erase(table);
            }
        
            void truncate(const std::string& table) {
                if (!session) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, "\n> Reason: No active session");
                if (!is_table_allowed(table)) throw Tool::Log::fetch("invalid-argument", Tool::Log::Type::Error, fmt::format("\n> Reason: Table '{}' — not defined", table));
                *session << fmt::format("TRUNCATE TABLE `{}`", table);
            }

            void alter(const std::string& table, const SchemaActions& actions) {
                if (!session) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, "\n> Reason: No active session");
                if (!is_table_allowed(table)) throw Tool::Log::fetch("invalid-argument", Tool::Log::Type::Error, fmt::format("\n> Reason: Table '{}' — not defined", table));
                if (actions.empty()) throw Tool::Log::fetch("invalid-argument", Tool::Log::Type::Error, "\n> Reason: Actions — cannot be empty");

                std::string sql = fmt::format("ALTER TABLE `{}` ", table);
                bool first = true;
                for (const auto& action : actions) {
                    if (!is_safe_identifier(action.column)) throw Tool::Log::fetch("invalid-argument", Tool::Log::Type::Error, fmt::format("\n> Reason: Column '{}' — invalid identifier", action.column));
                    if (!first) sql += ", ";
                    first = false;
                    switch (action.type) {
                        case SchemaAction::Type::Add: sql += fmt::format("ADD COLUMN {}", build_schema_definition(action.column, action.definition)); break;
                        case SchemaAction::Type::Drop: sql += fmt::format("DROP COLUMN `{}`", action.column); break;
                        case SchemaAction::Type::Modify: sql += fmt::format("MODIFY COLUMN {}", build_schema_definition(action.column, action.definition)); break;
                    }
                }
                *session << sql;

                auto& table_schema = schema[table];
                for (const auto& action : actions) {
                    switch (action.type) {
                        case SchemaAction::Type::Add:
                        case SchemaAction::Type::Modify:
                            table_schema[action.column] = action.definition;
                            break;
                        case SchemaAction::Type::Drop:
                            table_schema.erase(action.column);
                            break;
                    }
                }
            }

            Rows fetch(QueryBuilder* query) {
                if (!session) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, "\n> Reason: No active session");
                if (!is_table_allowed(query -> table)) throw Tool::Log::fetch("invalid-argument", Tool::Log::Type::Error, fmt::format("\n> Reason: Table '{}' — not defined", query -> table));
                
                std::string columns;
                if (query -> select.empty()) columns = "*";
                else {
                    for (int i = 0; i < (int)query -> select.size(); i++) {
                        if (!is_column_allowed(query -> table, query -> select[i])) throw Tool::Log::fetch("invalid-argument", Tool::Log::Type::Error, fmt::format("\n> Reason: Column '{}' — not defined in '{}'", query -> select[i], query -> table));
                        if (i > 0) columns += ", ";
                        columns += fmt::format("`{}`", query -> select[i]);
                    }
                }

                std::string sql = fmt::format("SELECT {} FROM `{}`", columns, query -> table);
                std::vector<std::string> binds, bind_names;
                query -> apply_where(sql, binds, bind_names);
                query -> apply_limit(sql);
                soci::row row_out;
                soci::statement st(*session);
                st.alloc();
                st.prepare(sql);
                for (int i = 0; i < (int)binds.size(); i++) {
                    st.exchange(soci::use(binds[i], bind_names[i]));
                }
                st.exchange(soci::into(row_out));
                st.define_and_bind();
                st.execute();

                Rows rows;
                while (st.fetch()) {
                    Row row;
                    for (std::size_t i = 0; i < row_out.size(); i++) {
                        const soci::column_properties& props = row_out.get_properties(i);
                        Tool::StackValue cell;
                        if (row_out.get_indicator(i) == soci::i_null) cell = nullptr;
                        else {
                            switch (props.get_data_type()) {
                                case soci::dt_string: cell = row_out.get<std::string>(i); break;
                                case soci::dt_integer: cell = static_cast<int32_t>(row_out.get<int>(i)); break;
                                case soci::dt_long_long: cell = static_cast<int64_t>(row_out.get<long long>(i)); break;
                                case soci::dt_double: cell = row_out.get<double>(i); break;
                                default: cell = row_out.get<std::string>(i); break;
                            }
                        }
                        row.emplace_back(props.get_name(), cell);
                    }
                    rows.push_back(row);
                }
                return rows;
            }

            bool execute(QueryBuilder* query) {
                assert_session_and_table(query -> table);
                std::string sql;
                std::vector<std::string> binds, bind_names;
                if (query -> query_type == "insert") {
                    std::string columns, placeholders;
                    bool first = true;
                    int index = 0;
                    for (const auto& [k, v] : query -> data) {
                        if (!is_column_allowed(query -> table, k)) throw Tool::Log::fetch("invalid-argument", Tool::Log::Type::Error, fmt::format("\n> Reason: Column '{}' — not defined in '{}'", k, query -> table));
                        if (!first) { columns += ", "; placeholders += ", "; }
                        first = false;
                        push_bind(query -> table, k, v, index++, columns, placeholders, binds, bind_names);
                    }
                    sql = fmt::format("INSERT INTO `{}` ({}) VALUES ({})", query -> table, columns, placeholders);
                }
                else if (query -> query_type == "delete") {
                    sql = fmt::format("DELETE FROM `{}`", query -> table);
                    query -> apply_where(sql, binds, bind_names);
                }
                else if (query -> query_type == "update") {
                    std::string sets;
                    bool first = true;
                    int index = 0;
                    for (const auto& [k, v] : query -> data) {
                        if (!is_column_allowed(query -> table, k)) throw Tool::Log::fetch("invalid-argument", Tool::Log::Type::Error, fmt::format("\n> Reason: Column '{}' — not defined in '{}'", k, query -> table));
                        if (!first) sets += ", ";
                        first = false;
                        std::string column, placeholder;
                        push_bind(query -> table, k, v, index++, column, placeholder, binds, bind_names);
                        sets += fmt::format("{} = {}", column, placeholder);
                    }
                    sql = fmt::format("UPDATE `{}` SET {}", query -> table, sets);
                    query -> apply_where(sql, binds, bind_names);
                }
                else throw Tool::Log::fetch("invalid-argument", Tool::Log::Type::Error, fmt::format("\n> Reason: Query type '{}' — unrecognized", query -> query_type));
                run_statement(sql, binds, bind_names, nullptr);
                return true;
            }
    };
}
#endif