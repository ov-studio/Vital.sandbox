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
    enum class Type { SBox, Info, Warn, Error };

    struct Command {
        std::string_view code;
        std::string_view message;
    };

    template <std::string_view const& L>
    struct Entry : std::runtime_error {
        using std::runtime_error::runtime_error;
        static constexpr std::string_view label = L;
    };

    inline constexpr std::string_view
        Label_sbox = "sbox",
        Label_info = "info",
        Label_warn = "warn", 
        Label_error = "error";

    using SBox = Entry<Label_sbox>;
    using Info = Entry<Label_info>;
    using Warn = Entry<Label_warn>;
    using Error = Entry<Label_error>;

    inline constexpr Command List[] = {
        {"invalid-argument", "invalid argument\n> Reason: {}"},
        {"request-failed", "request failed\n> Reason: {}"},
        {"invalid-thread", "invalid thread context\n> Reason: no active thread context found"}
    };

    template <typename... Types>
    inline bool is_type_impl(std::string_view label) { return (... || (label == Types::label)); }
    inline bool is_type(std::string_view label) {
        return is_type_impl<SBox, Info, Warn, Error>(label);
    }

    inline std::string_view resolve(std::string_view code) {
        for (const auto& e : List) if (code == e.code) return e.message;
        return "unknown error";
    }

    inline std::runtime_error fetch(std::string_view code, Type type = Type::Info, std::string_view detail = "") {
        auto body = fmt::format(fmt::runtime(resolve(code)), detail);
        switch (type) {
            case Type::SBox: return SBox(body);
            case Type::Info: return Info(body);
            case Type::Warn: return Warn(body);
            default: return Error(body);
        }
    }
}