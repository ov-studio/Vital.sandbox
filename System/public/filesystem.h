/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: filesystem.h
     Script: System: public: filesystem.h
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
#include <fstream>
#include <string>
#include <filesystem>


////////////////////////////
// Namespace: FileSystem //
////////////////////////////

namespace Vital::FileSystem {
    static const bool resolve(std::string& path);
    static const bool exists(std::string& path);
    static const std::streampos size(std::fstream handle);
    static const char* read(std::string& path);
    static const bool write(std::string& path, char* buffer, std::streampos size);
}