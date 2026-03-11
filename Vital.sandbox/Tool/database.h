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

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <Vital.sandbox/Tool/log.h>
#include <soci/soci.h>
#include <soci/mysql/soci-mysql.h>


////////////////////////////
// Vital: Tool: Database //
////////////////////////////

namespace Vital::Tool {
    class Database {
        public:
            struct Column {
                std::string type;
                bool nullable = true;
                bool primary = false;
                bool autoincrement = false;
            };

            struct QueryBuilder {
                Database* db;
                std::string table_name;
                std::string query_type;
                std::vector<std::string> select_cols;
                std::vector<std::tuple<std::string, std::string, std::string>> wheres;
                std::unordered_map<std::string, std::string> data;
                inline static const std::unordered_set<std::string> valid_ops = {"=", "!=", ">", "<", ">=", "<="};

                void destroy() {
                    delete this;
                }

                std::pair<std::string, std::vector<std::string>> build_where() {
                    std::string clause = " WHERE ";
                    std::vector<std::string> binds;
                    for (int i = 0; i < (int)wheres.size(); i++) {
                        const auto& [col, op, val] = wheres[i];
                        if (!db -> is_column_allowed(table_name, col) || !valid_ops.count(op)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                        if (i > 0) clause += " AND ";
                        clause += fmt::format("`{}` {} :w{}", col, op, i);
                        binds.push_back(val);
                    }
                    return {clause, binds};
                }

                void apply_where(std::string& sql, std::vector<std::string>& binds, std::vector<std::string>& bind_names) {
                    if (wheres.empty()) return;
                    auto [where_clause, where_binds] = build_where();
                    sql += where_clause;
                    for (int i = 0; i < (int)where_binds.size(); i++) {
                        bind_names.push_back(fmt::format("w{}", i));
                        binds.push_back(where_binds[i]);
                    }
                }
            };

            using TableSchema = std::unordered_map<std::string, Column>;
            using GlobalSchema = std::unordered_map<std::string, TableSchema>;
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

            bool is_column_allowed(const std::string& table, const std::string& col) const {
                auto it = schema.find(table);
                if (it == schema.end()) return false;
                return it -> second.find(col) != it -> second.end();
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

            static Database* create(const std::string& host, const std::string& user, const std::string& password, const std::string& db_name, unsigned int port = 3306) {
                auto db = new Database();
                std::string connection = fmt::format("host={} port={} user={} dbname={}", host, port, user, db_name);
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
                if (!is_safe_identifier(table)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                for (const auto& [col, def] : columns) {
                    if (!is_safe_identifier(col)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                }
                schema[table] = columns;
            }

            void sync() {
                if (!session) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Error);
                for (const auto& [table, columns] : schema) {
                    std::string sql = fmt::format("CREATE TABLE IF NOT EXISTS `{}` (", table);
                    std::string primary_key;
                    bool first = true;
                    for (const auto& [col, def] : columns) {
                        if (!first) sql += ", ";
                        first = false;
                        sql += fmt::format("`{}` {}", col, def.type);
                        if (def.autoincrement) sql += " AUTO_INCREMENT";
                        if (!def.nullable) sql += " NOT NULL";
                        if (def.primary) primary_key = col;
                    }
                    if (!primary_key.empty()) sql += fmt::format(", PRIMARY KEY (`{}`)", primary_key);
                    sql += ")";
                    *session << sql;
                }
            }
        
            QueryBuilder* table(const std::string& name) {
                if (!is_table_allowed(name)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto query = new QueryBuilder();
                query -> db = this;
                query -> table_name = name;
                return query;
            }

            void drop(const std::string& table) {
                if (!session) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Error);
                if (!is_table_allowed(table)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                *session << fmt::format("DROP TABLE IF EXISTS `{}`", table);
                schema.erase(table);
            }
        
            void truncate(const std::string& table) {
                if (!session) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Error);
                if (!is_table_allowed(table)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                *session << fmt::format("TRUNCATE TABLE `{}`", table);
            }

            std::vector<std::unordered_map<std::string, std::string>> fetch(QueryBuilder* query) {
                if (!session) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Error);
                if (!is_table_allowed(query -> table_name)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                
                std::string cols;
                if (query -> select_cols.empty()) cols = "*";
                else {
                    for (int i = 0; i < (int)query -> select_cols.size(); i++) {
                        if (!is_column_allowed(query -> table_name, query -> select_cols[i])) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                        if (i > 0) cols += ", ";
                        cols += fmt::format("`{}`", query -> select_cols[i]);
                    }
                }

                std::string sql = fmt::format("SELECT {} FROM `{}`", cols, query -> table_name);
                std::vector<std::string> binds, bind_names;
                query -> apply_where(sql, binds, bind_names);
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
                std::vector<std::unordered_map<std::string, std::string>> rows;
                while (st.fetch()) {
                    std::unordered_map<std::string, std::string> row_map;
                    for (std::size_t i = 0; i < row_out.size(); i++) {
                        const soci::column_properties& props = row_out.get_properties(i);
                        std::string val;
                        if (row_out.get_indicator(i) == soci::i_null) val = "";
                        else {
                            switch (props.get_data_type()) {
                                case soci::dt_string:    val = row_out.get<std::string>(i);               break;
                                case soci::dt_integer:   val = std::to_string(row_out.get<int>(i));       break;
                                case soci::dt_long_long: val = std::to_string(row_out.get<long long>(i)); break;
                                case soci::dt_double:    val = std::to_string(row_out.get<double>(i));    break;
                                default:                 val = row_out.get<std::string>(i);               break;
                            }
                        }
                        row_map[props.get_name()] = val;
                    }
                    rows.push_back(row_map);
                }
                return rows;
            }

            bool execute(QueryBuilder* query) {
                if (!session) throw Vital::Log::fetch("request-failed", Vital::Log::Type::Error);
                if (!is_table_allowed(query -> table_name)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);

                std::string sql;
                std::vector<std::string> binds, bind_names;
                if (query -> query_type == "insert") {
                    std::string cols, placeholders;
                    bool first = true;
                    int idx = 0;
                    for (const auto& [k, v] : query -> data) {
                        if (!is_column_allowed(query -> table_name, k)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                        if (!first) { cols += ", "; placeholders += ", "; }
                        first = false;
                        auto pname = fmt::format("d{}", idx++);
                        cols += fmt::format("`{}`", k);
                        placeholders += fmt::format(":{}", pname);
                        bind_names.push_back(pname);
                        binds.push_back(v);
                    }
                    sql = fmt::format("INSERT INTO `{}` ({}) VALUES ({})", query -> table_name, cols, placeholders);
                }
                else if (query -> query_type == "delete") {
                    sql = fmt::format("DELETE FROM `{}`", query -> table_name);
                    query -> apply_where(sql, binds, bind_names);
                }
                else if (query -> query_type == "update") {
                    std::string sets;
                    bool first = true;
                    int idx = 0;
                    for (const auto& [k, v] : query -> data) {
                        if (!is_column_allowed(query -> table_name, k)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                        if (!first) sets += ", ";
                        first = false;
                        auto pname = fmt::format("d{}", idx++);
                        sets += fmt::format("`{}` = :{}", k, pname);
                        bind_names.push_back(pname);
                        binds.push_back(v);
                    }
                    sql = fmt::format("UPDATE `{}` SET {}", query -> table_name, sets);
                    query -> apply_where(sql, binds, bind_names);
                }
                else throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                run_statement(sql, binds, bind_names, nullptr);
                return true;
            }
    };
}