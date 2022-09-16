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
#include <fstream>
#include <string>


////////////////////////////
// Namespace: FileSystem //
////////////////////////////

namespace Vital::FileSystem {
    const bool resolve(std::string& path);
    const bool exists(std::string& path);
    const std::streampos size(std::fstream handle);
    const char* read(std::string& path);
    const bool write(std::string& path, char* buffer, std::streampos size);
}