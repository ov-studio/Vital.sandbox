/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: index.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Type Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <pch.h>


////////////
// Vital //
////////////

namespace Vital {
    static const std::string Signature = "vsdk_v.0.0.1";

    struct Error {
        std::string_view code;
        std::string_view message;
    };

    static constexpr Error ErrorCode[] = {
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

    static inline const std::string get_error(std::string_view code, std::string message = "") {
        std::string_view error = "Unknown error";
        for (const auto& e : ErrorCode) {
            if (code == e.code) {
                error = e.message;
            }
        }
        return std::string(error);
    }
}

namespace Vital::Type {}
namespace Vital::System {}
namespace Vital::Sandbox {
    extern std::vector<std::string> fetchModules(const std::string& name);
}