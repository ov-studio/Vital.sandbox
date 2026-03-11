/*----------------------------------------------------------------
    Resource: Vital.sandbox
    Script: System: database.h
    Author: ov-studio
    Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
    DOC: 15/01/2026
    Desc: Database System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <soci/soci.h>
#include <soci/mysql/soci-mysql.h>


/////////////////////////////
// Vital: System: Database //
/////////////////////////////

namespace Vital::System {
    class Database {
        public:
            struct ColumnDef {
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
                std::pair<std::string, std::vector<std::string>> build_where() const;
            };

            using TableSchema = std::unordered_map<std::string, ColumnDef>;
            using GlobalSchema = std::unordered_map<std::string, TableSchema>;


        private:
            std::unique_ptr<soci::session> session;
            GlobalSchema schema;
            inline static const std::unordered_set<std::string> valid_ops = {"=", "!=", ">", "<", ">=", "<="};
            static bool is_safe_identifier(const std::string& name);
            static void validate_op(const std::string& op);
            bool is_table_allowed(const std::string& table) const;
            bool is_column_allowed(const std::string& table, const std::string& col) const;
            void run_statement(const std::string& sql, const std::vector<std::string>& binds, const std::vector<std::string>& bind_names, soci::row* row_out = nullptr);
        public:
            Database() = default;
            ~Database() = default;

            static Database* create(const std::string& host, const std::string& user, const std::string& password, const std::string& db_name, unsigned int port = 3306);
            void destroy();
            void define(const std::string& table, const TableSchema& columns);
            void sync();

            bool is_connected() const;
            QueryBuilder* table(const std::string& name);
            std::vector<std::unordered_map<std::string, std::string>> fetch(QueryBuilder* query);
            bool execute(QueryBuilder* query);
            void destroy_query(QueryBuilder* query);
    };
}