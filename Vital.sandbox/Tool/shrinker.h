/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: shrinker.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Shrinker Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/log.h>
#include <zstd.h>


////////////////////////////
// Vital: Tool: Shrinker //
////////////////////////////

namespace Vital::Tool::Shrinker {
    inline std::string compress(const std::string& input) {
        size_t dest_size = ZSTD_compressBound(input.size());
        std::string output;
        output.resize(dest_size);
        size_t res = ZSTD_compress(&output[0], dest_size, input.data(), input.size(), ZSTD_maxCLevel());
        if (ZSTD_isError(res)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, "Invalid compression");
        output.resize(res);
        return output;
    }

    inline std::string decompress(const std::string& input) {
        unsigned long long const frame_size = ZSTD_getFrameContentSize(input.data(), input.size());
        if (frame_size == ZSTD_CONTENTSIZE_ERROR || frame_size == ZSTD_CONTENTSIZE_UNKNOWN)
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, "Invalid decompression");
        std::string output;
        output.resize(static_cast<size_t>(frame_size));
        size_t res = ZSTD_decompress(&output[0], output.size(), input.data(), input.size());
        if (ZSTD_isError(res)) throw Tool::Log::fetch("request-failed", Tool::Log::Type::Error, "Invalid decompression");
        output.resize(res);
        return output;
    }
}