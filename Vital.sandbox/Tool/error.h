/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: error.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Error Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>


////////////
// Vital //
////////////

namespace Vital::Error {
    struct Command {
        std::string_view code;
        std::string_view message;
    };
    
    inline constexpr Command List[] = {
        {"invalid-result", "Invalid result"},
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
        {"file-busy", "File busy ('{}')"}
    };

    inline const std::runtime_error fetch(std::string_view code, std::string_view message = "") {
        std::string_view error = "Unknown error";
        for (const auto& e : List) {
            if (code == e.code) {
                error = e.message;
            }
        }
        return std::runtime_error(fmt::format(std::string(error), std::string(message)));
    }
}