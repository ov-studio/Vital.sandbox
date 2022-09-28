/*----------------------------------------------------------------
     Resource: Vital.sandbox
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
#include <pch.h>
#include <System/public/vital.h>


////////////////////////////
// Namespace: FileSystem //
////////////////////////////

namespace Vital::FileSystem {
    extern bool resolve(std::string& path);
    extern bool exists(std::string& path);
    extern std::streampos size(std::string& path);
    extern bool remove(std::string& path);
    extern std::string read(std::string& path);
    extern bool write(std::string& path, std::string buffer);
    extern std::vector<std::string> fetchContents(std::string& path);
}