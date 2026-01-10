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
        {"invalid-arguments", "Invalid argument list"},
        {"invalid-thread", "Invalid thread entity"},
        {"request-failed", "Failed to process request ('{}')"},
        {"hash-mode-nonexistent", "Failed to fetch hash mode ('{}')"},
        {"cipher-mode-nonexistent", "Failed to fetch cipher mode ('{}')"},
        {"cipher-invalid-key", "Invalid cipher key ('{}')"},
        {"cipher-invalid-iv", "Invalid cipher IV ('{}')"},
        {"file-nonexistent", "Failed to fetch file ('{}')"},
        {"serial-nonexistent", "Failed to fetch device's serial"}
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