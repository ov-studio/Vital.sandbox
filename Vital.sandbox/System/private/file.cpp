/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: private: file.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: File System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/System/public/file.h>


//////////////////////////
// Vital: System: File //
//////////////////////////

namespace Vital::System::File {
    bool resolve(std::string& path) {
        path = std::filesystem::absolute(path).string();
        return true;
    }

    bool exists(std::string& path) {
        resolve(path);
        return (std::filesystem::exists(path) && !std::filesystem::is_directory(path)) ? true : false;
    }

    std::streampos size(std::string& path) {
        if (!exists(path)) throw Vital::Error("file-nonexistent", path);
        return std::filesystem::file_size(path);
    }

    bool remove(std::string& path) {
        if (!exists(path)) throw Vital::Error("file-nonexistent", path);
        return std::filesystem::remove(path);
    }

    std::string read(std::string& path) {
        resolve(path);
        if (!exists(path)) throw Vital::Error("file-nonexistent", path);
        std::fstream handle(path, std::ios::in | std::ios::binary | std::ios::ate);
        auto size = std::filesystem::file_size(path);
        char* buffer = new char[size];
        handle.seekg(0, std::ios::beg);
        handle.read(buffer, size);
        handle.close();
        auto result = std::string(buffer, size);
        delete[] buffer;
        return result;
    }

    bool write(std::string& path, const std::string& buffer) {
        resolve(path);
        std::filesystem::create_directories(std::filesystem::path(path).parent_path().string());
        std::fstream handle(path, std::ios::out | std::ios::binary | std::ios::trunc);
        handle.write(buffer.data(), buffer.size());
        handle.close();
        return true;
    }

    std::vector<std::string> contents(std::string& path, bool fetchDirs) {
        resolve(path);
        std::vector<std::string> result;
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            auto& path = entry.path();
            bool isDir = std::filesystem::is_directory(path);
            if (fetchDirs == isDir) result.push_back(path.filename().string());
        }
        return result;
    }
}
