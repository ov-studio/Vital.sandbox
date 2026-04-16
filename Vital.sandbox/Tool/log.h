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
        Error,
        Warning
    };

    struct Command {
        std::string_view code;
        std::string_view message;
    };

    struct SBox : std::runtime_error {
        using std::runtime_error::runtime_error;
        static constexpr std::string_view label = "sbox";
    };

    struct Info : std::runtime_error {
        using std::runtime_error::runtime_error;
        static constexpr std::string_view label = "info";
    };

    struct Warning : std::runtime_error {
        using std::runtime_error::runtime_error;
        static constexpr std::string_view label = "warn";
    };

    struct Error : std::runtime_error {
        using std::runtime_error::runtime_error;
        static constexpr std::string_view label = "error";
    };

    inline constexpr Command List[] = {
        {"invalid-argument", "invalid argument {}"},
        {"request-failed", "request failed {}"},

        // TODO: UPDATE FOR ALL EM
        {"invalid-thread", "Invalid thread"},
        {"file-busy", "File busy (`{}`)"}
    };

    inline bool is_type(std::string_view label) {
        for (const auto& type : {
            std::string_view(SBox::label),
            std::string_view(Info::label),
            std::string_view(Warning::label),
            std::string_view(Error::label)
        }) if (label == type) return true;
        return false;
    }

    inline std::string_view resolve(std::string_view code) {
        for (const auto& e : List) {
            if (code == e.code) return e.message;
        }
        return "N/A";
    }

    inline std::runtime_error fetch(std::string_view code, Type type = Type::Info, std::string_view message = "") {
        auto formatted = fmt::format(std::string(resolve(code)), std::string(message));
        switch (type) {
            case Type::SBox: return SBox(formatted);
            case Type::Info: return Info(formatted);
            case Type::Warning: return Warning(formatted);
            default: return Error(formatted);
        }
    }
}