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
        path = std::filesystem::absolute(path).string();
        return true;
    }

    bool exists(std::string& path) {
        resolve(path);
        return (std::filesystem::exists(path) && !std::filesystem::is_directory(path)) ? true : false;
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
            auto bytes = std::filesystem::file_size(path);
            char* buffer = new char[(bytes + 1)];
            handle.seekg(0, std::ios::beg);
            handle.read(buffer, bytes);
            handle.close();
            buffer[bytes] = 0;
            result = buffer;
            delete[] buffer;
        }
        return result;
    }

    bool write(std::string& path, std::string buffer) {
        resolve(path);
        std::fstream handle(path, std::ios::out | std::ios::binary | std::ios::trunc);
        handle.write(buffer.c_str(), (buffer.size() + 1)*sizeof(char));
        handle.close();
        return true;
    }

    std::vector<std::string> fetchContents(std::string& path, bool fetchDirs) {
        resolve(path);
        std::vector<std::string> result;
        for (auto& entry : std::filesystem::directory_iterator(path)) {
            auto& path = entry.path();
            bool isDir = std::filesystem::is_directory(path);
            if (fetchDirs == isDir) result.push_back(path.filename().string());
        }
        return result;
    }
}
