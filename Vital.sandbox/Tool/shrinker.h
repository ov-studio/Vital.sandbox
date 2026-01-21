/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: shrinker.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Crypto Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/error.h>
#include <zlib.h>


////////////////////////////
// Vital: Tool: Shrinker //
////////////////////////////

namespace Vital::Tool::Shrinker {
    inline std::string compress(const std::string& input) {
        uLong src_size = static_cast<uLong>(input.size());
        uLong dest_size = compressBound(src_size);
        std::string output;
        output.resize(dest_size);
        int res = compress2(reinterpret_cast<Bytef*>(&output[0]), &dest_size, reinterpret_cast<const Bytef*>(input.data()), src_size, Z_BEST_COMPRESSION);
        if (res != Z_OK) throw std::runtime_error("zlib compression failed");
        output.resize(dest_size);
        return output;
    }
    
    inline std::string decompress(const std::string& input) {
        uLong dest_size = input.size()*4;
        std::string output;
        output.resize(dest_size);
        int res;
        while ((res = uncompress(reinterpret_cast<Bytef*>(&output[0]), &dest_size, reinterpret_cast<const Bytef*>(input.data()), static_cast<uLong>(input.size()))) == Z_BUF_ERROR) {
            dest_size *= 2;
            output.resize(dest_size);
        }
        if (res != Z_OK) throw std::runtime_error("zlib decompression failed");
        output.resize(dest_size);
        return output;
    }
}