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
#include <Vital.sandbox/Tool/log.h>
#include <ryml.hpp>
#include <ryml_std.hpp>


////////////////////////
// Vital: Tool: YAML //
////////////////////////

namespace Vital::Tool::YAML {
    // TOOD: Improve

    // Scoped ryml error handler — replaces std::abort() with a throw
    // so malformed input never terminates the process.
    // loc.line/col are included when available (ryml may omit them
    // for pre-positional errors such as empty input).
    struct ErrorScope {
        ryml::Callbacks prev;

        ErrorScope() {
            prev = ryml::get_callbacks();
            ryml::Callbacks cb = prev;
            cb.m_error = [](const char* msg, size_t len, ryml::Location loc, void*) {
                std::string detail = std::string(msg, len);
                if (loc.line > 0)
                    detail += " (line " + std::to_string(loc.line) + ", col " + std::to_string(loc.col) + ")";
                throw std::runtime_error(detail);
            };
            ryml::set_callbacks(cb);
        }

        ~ErrorScope() {
            ryml::set_callbacks(prev);
        }
    };

    // Parse a YAML string into a ryml::Tree.
    // Throws Vital::Log on malformed input or if root is not a map.
    // Error messages include line/col when ryml can provide them.
    inline ryml::Tree parse(const std::string& input) {
        ErrorScope scope;
        ryml::Tree tree;
        try {
            tree = ryml::parse_in_arena(ryml::to_csubstr(input));
        }
        catch (const std::runtime_error& e) {
            throw Vital::Log::fetch("request-failed", Vital::Log::Type::Error,
                std::string("Invalid YAML: ") + e.what());
        }
        catch (...) {
            throw Vital::Log::fetch("request-failed", Vital::Log::Type::Error, "Invalid YAML");
        }
        if (!tree.rootref().is_map())
            throw Vital::Log::fetch("request-failed", Vital::Log::Type::Error, "YAML root is not a map");
        return tree;
    }

    // Safely check whether a key exists in a map node.
    inline bool has(ryml::ConstNodeRef node, const char* key) {
        return node.is_map() && node.has_child(ryml::to_csubstr(key));
    }

    // Safely read a string scalar from a map node by key.
    // Returns fallback if the key is absent or the node is not a scalar.
    inline std::string get_str(ryml::ConstNodeRef node, const char* key, const std::string& fallback = "") {
        if (!node.is_map()) return fallback;
        if (!node.has_child(ryml::to_csubstr(key))) return fallback;
        auto child = node[ryml::to_csubstr(key)];
        if (!child.is_keyval() && !child.is_val()) return fallback;
        std::string out;
        child >> out;
        return out;
    }

    // Safely read a bool scalar from a map node by key.
    // Accepts: true/false, yes/no, 1/0.
    // Returns fallback if the key is absent or the value is not a valid bool.
    inline bool get_bool(ryml::ConstNodeRef node, const char* key, bool fallback = false) {
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

    // Safely read an int scalar from a map node by key.
    // Returns fallback if the key is absent or the value is not a valid integer.
    inline int get_int(ryml::ConstNodeRef node, const char* key, int fallback = 0) {
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

    // Safely read a float scalar from a map node by key.
    // Returns fallback if the key is absent or the value is not a valid float.
    inline float get_float(ryml::ConstNodeRef node, const char* key, float fallback = 0.0f) {
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
}