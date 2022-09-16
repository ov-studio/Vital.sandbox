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
#include <iostream>
#include <filesystem>
#include "System/public/filesystem.h"


////////////////////////////
// Namespace: FileSystem //
////////////////////////////

namespace Vital::FileSystem {
    const bool resolve(std::string& path) {
        std::filesystem::path __path = std::filesystem::absolute(path);
        if (!path.empty()) path = __path.string();
        return true
    }

    const bool exists(std::string& path) {
        resolve(path);
        std::fstream handle(path, std::ios::in | std::ios::binary);
        const bool isValid = (handle.is_open() && true) || false;
        handle.close();
        return isValid;
    }

    const std::streampos size(std::fstream handle) {
        if (!handle.is_open()) return 0;
        handle.seekg(0, std::ios::end);
        return handle.tellg();
    }

    const char* read(std::string& path) {
        resolve(path);
        std::fstream handle(path, std::ios::in | std::ios::binary | std::ios::ate);
        char* buffer = nullptr;
        if (handle.is_open()) {
            const std::streampos size = handle.tellg();
            char* buffer = new char[size];
            handle.seekg(0, std::ios::beg);
            handle.read(buffer, size);
        }
        return buffer;
    }

    const bool write(std::string& path, char* buffer, std::streampos size) {
        resolve(path);
        std::fstream handle(path, std::ios::out | std::ios::binary | std::ios::trunc);
        handle.write(buffer, size);
        handle.close();
        return true;
    }
}