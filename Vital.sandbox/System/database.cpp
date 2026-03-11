/*----------------------------------------------------------------
    Resource: Vital.sandbox
    Script: System: database.cpp
    Author: ov-studio
    Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
    DOC: 15/01/2026
    Desc: Database System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/System/database.h>


/////////////////////////////
// Vital: Engine: Database //
/////////////////////////////

namespace Vital::System {
    // Helpers //
    bool Database::is_safe_identifier(const std::string& name) {
        if (name.empty() || name.size() > 64) return false;
        for (char c : name) {
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') return false;
        }
        return true;
    }

    bool Database::is_table_allowed(const std::string& table) const {
        return schema.find(table) != schema.end();
    }

    bool Database::is_column_allowed(const std::string& table, const std::string& col) const {
        auto it = schema.find(table);
        if (it == schema.end()) return false;
        return it->second.find(col) != it->second.end();
    }

    void Database::validate_op(const std::string& op) {
        if (!valid_ops.count(op)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
    }


    // Instantiators //
    Database* Database::create(const std::string& host, const std::string& user, const std::string& password, const std::string& db_name, unsigned int port) {
        auto* db = new Database();
        std::string conn = fmt::format("host={} port={} user={} dbname={}", host, port, user, db_name);
        if (!password.empty()) conn += fmt::format(" password={}", password);
        db -> session = std::make_unique<soci::session>(soci::mysql, conn);
        return db;
    }

    void Database::destroy() {
        disconnect();
        delete this;
    }


    // Managers //
    void Database::disconnect() {
        if (!session) return;
        session -> close();
        session.reset();
    }

    void Database::define(const std::string& table, const TableSchema& columns) {
        if (!is_safe_identifier(table)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
        for (const auto& [col, def] : columns) {
            if (!is_safe_identifier(col)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
        }
        schema[table] = columns;
    }

    void Database::sync() {
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


    // Checkers //
    bool Database::is_connected() const {
        return session != nullptr && session -> is_connected();
    }


    // QueryBuilder //
    std::pair<std::string, std::vector<std::string>> Database::QueryBuilder::build_where() const {
        std::string clause = " WHERE ";
        std::vector<std::string> binds;
        for (int i = 0; i < (int)wheres.size(); i++) {
            const auto& [col, op, val] = wheres[i];
            if (!db -> is_column_allowed(table_name, col)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
            Database::validate_op(op);
            if (i > 0) clause += " AND ";
            clause += fmt::format("`{}` {} :w{}", col, op, i);
            binds.push_back(val);
        }
        return {clause, binds};
    }


    // Query API //
    Database::QueryBuilder* Database::table(const std::string& name) {
        if (!is_table_allowed(name)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
        auto* query = new QueryBuilder();
        query -> db = this;
        query -> table_name = name;
        return query;
    }

    std::vector<std::unordered_map<std::string, std::string>> Database::fetch(QueryBuilder* query) {
        if (!session) throw Vital::Log::fetch("request-failed",   Vital::Log::Type::Error);
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
        std::vector<std::string> binds;
        if (!query -> wheres.empty()) {
            auto [where_clause, where_binds] = query -> build_where();
            sql += where_clause;
            binds = where_binds;
        }

        soci::row row_out;
        soci::statement st(*session);
        st.alloc();
        st.prepare(sql);
        for (int i = 0; i < (int)binds.size(); i++) {
            st.exchange(soci::use(binds[i], fmt::format("w{}", i)));
        }
        st.exchange(soci::into(row_out));
        st.define_and_bind();
        st.execute();

        std::vector<std::unordered_map<std::string, std::string>> rows;
        while (st.fetch()) {
            const soci::row& r = row_out;
            std::unordered_map<std::string, std::string> row_map;
            for (std::size_t i = 0; i < r.size(); i++) {
                const soci::column_properties& props = r.get_properties(i);
                std::string val;
                if (r.get_indicator(i) == soci::i_null) val = "";
                else {
                    // convert all types to string for Lua
                    switch (props.get_data_type()) {
                        case soci::dt_string: val = r.get<std::string>(i); break;
                        case soci::dt_integer: val = std::to_string(r.get<int>(i)); break;
                        case soci::dt_long_long: val = std::to_string(r.get<long long>(i)); break;
                        case soci::dt_double: val = std::to_string(r.get<double>(i)); break;
                        default: val = r.get<std::string>(i); break;
                    }
                }
                row_map[props.get_name()] = val;
            }
            rows.push_back(row_map);
        }
        return rows;
    }

    bool Database::execute(QueryBuilder* query) {
        if (!session) throw Vital::Log::fetch("request-failed",   Vital::Log::Type::Error);
        if (!is_table_allowed(query -> table_name)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);

        std::string sql;
        std::vector<std::string> binds;
        std::vector<std::string> bind_names;

        if (query -> query_type == "insert") {
            std::string cols, placeholders;
            bool first = true;
            int  idx   = 0;
            for (const auto& [k, v] : query -> data) {
                if (!is_column_allowed(query -> table_name, k)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                if (!first) { cols += ", "; placeholders += ", "; }
                first = false;
                auto pname = fmt::format("d{}", idx++);
                cols         += fmt::format("`{}`", k);
                placeholders += fmt::format(":{}", pname);
                bind_names.push_back(pname);
                binds.push_back(v);
            }
            sql = fmt::format("INSERT INTO `{}` ({}) VALUES ({})", query -> table_name, cols, placeholders);
        }
        else if (query -> query_type == "update") {
            std::string sets;
            bool first = true;
            int  idx   = 0;
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
            if (!query -> wheres.empty()) {
                auto [where_clause, where_binds] = query -> build_where();
                sql += where_clause;
                for (int i = 0; i < (int)where_binds.size(); i++) {
                    bind_names.push_back(fmt::format("w{}", i));
                    binds.push_back(where_binds[i]);
                }
            }
        }
        else if (query -> query_type == "delete") {
            sql = fmt::format("DELETE FROM `{}`", query -> table_name);
            if (!query -> wheres.empty()) {
                auto [where_clause, where_binds] = query -> build_where();
                sql += where_clause;
                for (int i = 0; i < (int)where_binds.size(); i++) {
                    bind_names.push_back(fmt::format("w{}", i));
                    binds.push_back(where_binds[i]);
                }
            }
        }
        else throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);

        soci::statement st(*session);
        st.alloc();
        st.prepare(sql);
        for (int i = 0; i < (int)binds.size(); i++) {
            st.exchange(soci::use(binds[i], bind_names[i]));
        }
        st.define_and_bind();
        st.execute(true);
        return true;
    }

    void Database::destroy_query(QueryBuilder* query) {
        delete query;
    }
}