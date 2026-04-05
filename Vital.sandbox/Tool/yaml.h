/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: yaml.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: YAML Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <ryml.hpp>
#include <ryml_std.hpp>


////////////////////////
// Vital: Tool: YAML //
////////////////////////

namespace Vital::Tool {
    class YAML {
        private:
            ryml::Tree tree;
            ryml::ConstNodeRef root;

            struct ErrorScope {
                ryml::Callbacks prev;

                ErrorScope() {
                    prev = ryml::get_callbacks();
                    ryml::Callbacks cb = prev;
                    cb.m_error = [](const char*, size_t, ryml::Location loc, void*) {
                        std::string detail = "parse error";
                        if (loc.line > 0)
                            detail += " at line " + std::to_string(loc.line) + ", col " + std::to_string(loc.col);
                        throw std::runtime_error(detail);
                    };
                    ryml::set_callbacks(cb);
                }

                ~ErrorScope() {
                    ryml::set_callbacks(prev);
                }
            };

        public:
            YAML() = default;

            YAML(const YAML&) = delete;
            YAML& operator=(const YAML&) = delete;
            YAML(YAML&&) = default;
            YAML& operator=(YAML&&) = default;

            void parse(const std::string& input) {
                ErrorScope scope;
                try {
                    tree = ryml::parse_in_arena(ryml::to_csubstr(input));
                }
                catch (const std::runtime_error& e) {
                    throw std::runtime_error(e.what());
                }
                catch (...) {
                    throw std::runtime_error("unknown");
                }
                root = tree.rootref();
                if (!root.is_map()) throw std::runtime_error("root is not a map");
            }

            ryml::ConstNodeRef get_root() const {
                return root;
            }

            static bool has(ryml::ConstNodeRef node, const char* key) {
                return node.is_map() && node.has_child(ryml::to_csubstr(key));
            }

            bool has(const char* key) const {
                return root.is_map() && root.has_child(ryml::to_csubstr(key));
            }

            static std::string get_str(ryml::ConstNodeRef node, const char* key, const std::string& fallback = "") {
                if (!node.is_map()) return fallback;
                if (!node.has_child(ryml::to_csubstr(key))) return fallback;
                auto child = node[ryml::to_csubstr(key)];
                if (!child.is_keyval() && !child.is_val()) return fallback;
                std::string out;
                child >> out;
                return out;
            }

            std::string get_str(const char* key, const std::string& fallback = "") const {
                return get_str(root, key, fallback);
            }

            static bool get_bool(ryml::ConstNodeRef node, const char* key, bool fallback = false) {
                if (!node.is_map()) return fallback;
                if (!node.has_child(ryml::to_csubstr(key))) return fallback;
                auto child = node[ryml::to_csubstr(key)];
                if (!child.is_keyval() && !child.is_val()) return fallback;
                std::string raw;
                child >> raw;
                if (raw == "true"  || raw == "yes" || raw == "1") return true;
                if (raw == "false" || raw == "no"  || raw == "0") return false;
                return fallback;
            }

            bool get_bool(const char* key, bool fallback = false) const {
                return get_bool(root, key, fallback);
            }

            static int get_int(ryml::ConstNodeRef node, const char* key, int fallback = 0) {
                if (!node.is_map()) return fallback;
                if (!node.has_child(ryml::to_csubstr(key))) return fallback;
                auto child = node[ryml::to_csubstr(key)];
                if (!child.is_keyval() && !child.is_val()) return fallback;
                try {
                    std::string raw;
                    child >> raw;
                    return std::stoi(raw);
                }
                catch (...) { return fallback; }
            }

            int get_int(const char* key, int fallback = 0) const {
                return get_int(root, key, fallback);
            }

            static float get_float(ryml::ConstNodeRef node, const char* key, float fallback = 0.0f) {
                if (!node.is_map()) return fallback;
                if (!node.has_child(ryml::to_csubstr(key))) return fallback;
                auto child = node[ryml::to_csubstr(key)];
                if (!child.is_keyval() && !child.is_val()) return fallback;
                try {
                    std::string raw;
                    child >> raw;
                    return std::stof(raw);
                }
                catch (...) { return fallback; }
            }

            float get_float(const char* key, float fallback = 0.0f) const {
                return get_float(root, key, fallback);
            }
    };
}