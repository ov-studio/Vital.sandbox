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
#include <System/public/vital.h>


////////////////////////////
// Namespace: FileSystem //
////////////////////////////

namespace Vital::FileSystem {
    bool resolve(std::string& path);
    bool exists(std::string& path);
    std::streampos size(std::fstream handle);
    bool remove(std::string& path);
    char* read(std::string& path);
    bool write(std::string& path, char* buffer);
}