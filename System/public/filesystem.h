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
    static bool resolve(std::string& path);
    static bool exists(std::string& path);
    static std::streampos size(std::string& path);
    static bool remove(std::string& path);
    static char* read(std::string& path);
    static bool write(std::string& path, char* buffer);
}