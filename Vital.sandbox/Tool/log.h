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
    enum class Type { 
        SBox, 
        Info,
        Warning, 
        Error 
    };

    struct Command {
        std::string_view code;
        std::string_view message;
    };

    template <std::string_view const& Label>
    struct Entry : std::runtime_error {
        using std::runtime_error::runtime_error;
        static constexpr std::string_view label = Label;
    };

    inline constexpr std::string_view
        LabelSBox = "sbox",
        LabelInfo = "info",
        LabelWarning = "warn",
        LabelError = "error";

    using SBox = Entry<LabelSBox>;
    using Info = Entry<LabelInfo>;
    using Warning = Entry<LabelWarning>;
    using Error = Entry<LabelError>;

    inline constexpr Command List[] = {
        {"invalid-argument", "invalid argument\n> Reason: {}"},
        {"request-failed", "request failed\n> Reason: {}"},
        {"invalid-thread", "invalid thread context\n> Reason: no active thread context found"},
    };

    inline std::string_view resolve(std::string_view code) {
        for (const auto& e : List) if (code == e.code) return e.message;
        return "unknown error";
    }

    inline std::runtime_error fetch(std::string_view code, Type type = Type::Info, std::string_view detail = "") {
        auto body = fmt::format(fmt::runtime(resolve(code)), detail);
        switch (type) {
            case Type::SBox: return SBox(body);
            case Type::Info: return Info(body);
            case Type::Warning: return Warning(body);
            default: return Error(body);
        }
    }
}