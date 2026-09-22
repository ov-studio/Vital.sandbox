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

    template <Type T>
    struct Entry : std::runtime_error {
        using std::runtime_error::runtime_error;
        static constexpr std::string_view label = type_labels[static_cast<std::size_t>(T)];
    };

    using sbox = Entry<Type::sbox>;
    using info = Entry<Type::info>;
    using warn = Entry<Type::warn>;
    using error = Entry<Type::error>;


    // Helpers //
    inline constexpr std::string_view type_label(Type t) {
        return type_labels[static_cast<std::size_t>(t)];
    }

    inline bool is_type(std::string_view label) {
        for (const auto& l : type_labels) if (l == label) return true;
        return false;
    }

    inline bool is_runtime_level(std::string_view label) {
        return label != type_label(Type::sbox) && is_type(label);
    }

    inline std::runtime_error fetch(std::string_view code, Type type = Type::info, std::string_view detail = "") {
        std::string body(code);
        if (!detail.empty()) body += fmt::format("\n> Reason: {}", detail);
        switch (type) {
            case Type::sbox: return sbox(body);
            case Type::info: return info(body);
            case Type::warn: return warn(body);
            default:         return error(body);
        }
    }
}