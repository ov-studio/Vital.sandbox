/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: log.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Log Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <fmt/core.h>


////////////
// Vital //
////////////

namespace Vital::Tool::Log {
    enum class Type : std::size_t { 
        sbox, 
        info, 
        warn, 
        error 
    };

    inline constexpr std::string_view type_labels[] = { "sbox", "info", "warn", "error" };

    struct Command {
        std::string_view code;
        std::string_view message;
    };

    inline constexpr Command error_list[] = {
        { "invalid-argument", "invalid argument {}" },
        { "request-failed",   "request failed {}"   },
    };

    template <Type T>
    struct Entry : std::runtime_error {
        using std::runtime_error::runtime_error;
        static constexpr std::string_view label = L;
        static constexpr std::string_view label = type_labels[static_cast<std::size_t>(T)];
    };

    using sbox = Entry<Type::sbox>;
    using info = Entry<Type::info>;
    using warn = Entry<Type::warn>;
    using error = Entry<Type::error>;

    inline constexpr Command error_list[] = {
        {"invalid-argument", "invalid argument {}"},
        {"request-failed", "request failed {}"}
    };

    inline bool is_type(std::string_view label) {
        for (const auto& l : type_labels) if (l == label) return true;
        return false;
    }

    inline bool is_runtime_level(std::string_view label) {
        return runtime_levels.count(label) > 0;
    }

    inline std::string_view resolve(std::string_view code) {
        for (const auto& e : error_list) if (code == e.code) return e.message;
        return "unknown error";
    }

    inline std::runtime_error fetch(std::string_view code, Type type = Type::info, std::string_view detail = "") {
        auto body = fmt::format(fmt::runtime(resolve(code)), detail);
        switch (type) {
            case Type::sbox: return sbox(body);
            case Type::info: return info(body);
            case Type::warn: return warn(body);
            default:         return error(body);
        }
    }
}