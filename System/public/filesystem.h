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
    extern __declspec(dllexport) bool resolve(std::string& path);
    extern __declspec(dllexport) bool exists(std::string& path);
    extern __declspec(dllexport) std::streampos size(std::string& path);
    extern __declspec(dllexport) bool remove(std::string& path);
    extern __declspec(dllexport) char* read(std::string& path);
    extern __declspec(dllexport) bool write(std::string& path, char* buffer);
}