/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: format.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Format Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <Vital.sandbox/Tool/file.h>


//////////////////////////
// Vital: Tool: Format //
/////////////////////////

namespace Vital::Tool::Format {
    template<typename F>
    struct Descriptor {
        F format;
        std::string extension;
        std::vector<uint8_t> magic_bytes;
    };

    template<typename F>
    inline int max_magic_size(const std::vector<Descriptor<F>>& registry) {
        int max = 0;
        for (const auto& desc : registry)
            max = std::max(max, static_cast<int>(desc.magic_bytes.size()));
        return max;
    }

    template<typename F>
    inline bool is_supported_extension(const std::vector<Descriptor<F>>& registry, const std::string& path) {
        const size_t dot = path.rfind('.');
        if (dot == std::string::npos) return false;
        std::string ext = path.substr(dot + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        for (const auto& desc : registry) {
            if (desc.extension == ext) return true;
        }
        return false;
    }

    template<typename F>
    inline F get_format(const std::vector<Descriptor<F>>& registry, F unknown, const uint8_t* ptr, int size) {
        for (const auto& desc : registry) {
            const int magic_size = static_cast<int>(desc.magic_bytes.size());
            if (size < magic_size) continue;
            if (std::equal(desc.magic_bytes.begin(), desc.magic_bytes.end(), ptr)) return desc.format;
        }
        return unknown;
    }

    template<typename F>
    inline F get_format(const std::vector<Descriptor<F>>& registry, F unknown, const godot::PackedByteArray& buffer) {
        return get_format(registry, unknown, buffer.ptr(), buffer.size());
    }

    template<typename F>
    inline F get_format(const std::vector<Descriptor<F>>& registry, F unknown, const std::string& path) {
        if (!is_supported_extension(registry, path)) return unknown;
        const auto magic = Tool::File::read_magic(Tool::get_directory(), path, max_magic_size(registry));
        return get_format(registry, unknown, magic.ptr(), magic.size());
    }

    template<typename F>
    inline bool is_supported_format(const std::vector<Descriptor<F>>& registry, F unknown, const std::string& path) {
        return get_format(registry, unknown, path) != unknown;
    }
}