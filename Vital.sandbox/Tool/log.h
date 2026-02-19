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
#include <Vendor/fmt/core.h>


////////////
// Vital //
////////////

namespace Vital::Log {
    enum class Type {
        Info,
        Error,
        Warning
    };

    struct Command {
        std::string_view code;
        std::string_view message;
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
        {"invalid-arguments", "Invalid arguments"},
        {"invalid-thread", "Invalid thread"},
        {"request-failed", "Request failed ('{}')"},
        {"serial-nonexistent", "Device serial not found"},
        {"hash-mode-nonexistent", "Hash mode not found ('{}')"},
        {"cipher-mode-nonexistent", "Cipher mode not found ('{}')"},
        {"cipher-invalid-key", "Invalid cipher key ('{}')"},
        {"cipher-invalid-iv", "Invalid cipher IV ('{}')"},
        {"base-path-invalid", "Invalid base path ('{}')"},
        {"file-path-invalid", "Invalid file path ('{}')"},
        {"file-nonexistent", "File not found ('{}')"},
        {"directory-nonexistent", "Directory not found ('{}')"},
        {"file-busy", "File busy ('{}')"},
        {"webview-failed", "Webview failed ('{}')"}
    };

    inline std::string_view resolve(std::string_view code) {
        for (const auto& e : List) {
            if (code == e.code) 
                return e.message;
        }
        return "N/A";
    }

    inline std::runtime_error fetch(std::string_view code, Type type = Type::Info, std::string_view message = "") {
        auto formatted = fmt::format(std::string(resolve(code)), std::string(message));
        switch (type) {
            case Type::Warning: return Warning(formatted);
            default: return Error(formatted);
        }
    }
}