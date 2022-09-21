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
    extern bool resolve(std::string& path);
    extern bool exists(std::string& path);
    extern std::streampos size(std::string& path);
    extern bool remove(std::string& path);
    extern char* read(std::string& path);
    extern bool write(std::string& path, char* buffer);
}