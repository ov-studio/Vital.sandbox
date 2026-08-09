/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: version.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vital Version
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Vital/pch.h>

// Injected by CI from git tag — do not edit manually
#ifndef VSDK_VERSION_MAJOR
#define VSDK_VERSION_MAJOR 0
#endif
#ifndef VSDK_VERSION_MINOR
#define VSDK_VERSION_MINOR 0
#endif
#ifndef VSDK_VERSION_PATCH
#define VSDK_VERSION_PATCH 0
#endif
#ifndef VSDK_VERSION_LABEL
#define VSDK_VERSION_LABEL "dev"
#endif
#ifndef VSDK_WRY_VERSION
#define VSDK_WRY_VERSION "unknown"
#endif
#ifndef VSDK_GODOT_VERSION
#define VSDK_GODOT_VERSION "unknown"
#endif


///////////////////////////
// Vital: Tool: Version //
///////////////////////////

namespace Vital {
    namespace Manager::Kit {
        const std::string& get_version();
    }
    
    namespace Tool {
        namespace Version {
            struct Info {
                int major;
                int minor;
                int patch;
                std::string label;

                std::string to_string() const {
                    std::string v = "v" + std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
                    if (!label.empty()) v += "-" + label;
                    return v;
                }

                int get_major() const { return major; }
                int get_minor() const { return minor; }
                int get_patch() const { return patch; }
                std::string get_label() const { return label; }
                std::string get_version() const { return to_string(); }
                bool is_prerelease() const { return !label.empty(); }
                bool is_at_least(int maj, int min = 0, int pat = 0) const { return *this >= Info{ maj, min, pat, "" }; }

                bool operator==(const Info& o) const { return major == o.major && minor == o.minor && patch == o.patch && label == o.label; }
                bool operator< (const Info& o) const {
                    if (major != o.major) return major < o.major;
                    if (minor != o.minor) return minor < o.minor;
                    if (patch != o.patch) return patch < o.patch;
                    if (label.empty() != o.label.empty()) return !label.empty();
                    return label < o.label;
                }
                bool operator> (const Info& o) const { return o < *this; }
                bool operator<=(const Info& o) const { return !(o < *this); }
                bool operator>=(const Info& o) const { return !(*this < o); }
            };

            static const Info SDK = {
                VSDK_VERSION_MAJOR,
                VSDK_VERSION_MINOR,
                VSDK_VERSION_PATCH,
                VSDK_VERSION_LABEL
            };

            inline std::string get(const std::string& type) {
                std::string key = type;
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                if (key == "sdk")   return SDK.to_string();
                if (key == "kit")   return Manager::Kit::get_version();
                if (key == "wry")   return VSDK_WRY_VERSION;
                if (key == "godot") return VSDK_GODOT_VERSION;
                return "unknown";
            }
        }
    }
}