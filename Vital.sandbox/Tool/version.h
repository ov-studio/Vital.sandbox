/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: index.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vital Tools
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
    struct Version {
        int major;
        int minor;
        int patch;
        std::string label; // optional: pre-release label e.g. "alpha", "beta", ""

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
        bool is_at_least(int maj, int min = 0, int pat = 0) const { return *this >= Version{ maj, min, pat, "" }; }

        bool operator==(const Version& o) const { return major == o.major && minor == o.minor && patch == o.patch && label == o.label; }
        bool operator< (const Version& o) const {
            if (major != o.major) return major < o.major;
            if (minor != o.minor) return minor < o.minor;
            if (patch != o.patch) return patch < o.patch;
            if (label.empty() != o.label.empty()) return !label.empty();
            return label < o.label;
        }
        bool operator> (const Version& o) const { return o < *this; }
        bool operator<=(const Version& o) const { return !(o < *this); }
        bool operator>=(const Version& o) const { return !(*this < o); }
    };
}