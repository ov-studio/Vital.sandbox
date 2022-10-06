/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: file.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: File System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/vital.h>


//////////////////////////
// Vital: System: File //
//////////////////////////

namespace Vital::System::File {
    extern bool resolve(std::string& path);
    extern bool exists(std::string& path);
    extern std::streampos size(std::string& path);
    extern bool remove(std::string& path);
    extern std::string& read(std::string& path);
    extern bool write(std::string& path, const std::string& buffer);
    extern std::vector<std::string>& contents(std::string& path, bool fetchDirs = false);
}