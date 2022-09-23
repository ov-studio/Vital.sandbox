/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: private: filesystem.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: File System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/filesystem.h>


////////////////////////////
// Namespace: FileSystem //
////////////////////////////

namespace Vital::FileSystem {
    bool resolve(std::string& path) {
        std::filesystem::path __path = std::filesystem::absolute(path);
        path = !__path.empty() ? __path.string() : path;
        return true;
    }

    bool exists(std::string& path) {
        resolve(path);
        return std::filesystem::exists(path);
    }

    std::streampos size(std::string& path) {
        if (!exists(path)) return 0;
        return std::filesystem::file_size(path);
    }

    bool remove(std::string& path) {
        if (!exists(path)) return false;
        return std::filesystem::remove(path);
    }

    std::string read(std::string& path) {
        resolve(path);
        std::string result = "";
        if (exists(path)) {
            std::fstream handle(path, std::ios::in | std::ios::binary | std::ios::ate);
            std::streampos bytes = size(path);
            char* buffer = new char[bytes];
            handle.seekg(0, std::ios::beg);
            handle.read(buffer, bytes);
            handle.close();
            result = buffer;
            delete buffer;
        }
        return result;
    }

    bool write(std::string& path, std::string buffer) {
        resolve(path);
        std::fstream handle(path, std::ios::out | std::ios::binary | std::ios::trunc);
        handle.write(buffer.c_str(), sizeof(buffer));
        handle.close();
        return true;
    }
}
