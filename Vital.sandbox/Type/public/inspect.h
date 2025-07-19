/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: inspect.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Inspect Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/index.h>


///////////////////////////
// Vital: Type: Inspect //
///////////////////////////

namespace Vital::Type::Inspect {
    typedef struct {
        const std::wstring name;
        const std::wstring model;
        const std::wstring manufacturer;
        const std::wstring hardware_id;
        const std::wstring os;
        const std::wstring os_architecture;
        const std::wstring os_version;
        const std::wstring os_serial;
    } System;

    typedef struct {
        const std::wstring manufacturer;
        const std::wstring product;
        const std::wstring version;
        const std::wstring serial;
    } SMBIOS;

    typedef struct {
        const std::wstring name;
        const std::wstring manufacturer;
        const std::wstring id;
        const int cores;
        const int threads;
    } CPU;

    typedef struct {
        const std::wstring name;
        const std::wstring version;
        const int refresh_rate;
        const std::pair<int, int> resolution;
        const unsigned long long capacity;
    } GPU;
            
    typedef struct {
        const std::wstring manufacturer;
        const std::wstring version;
        const std::wstring serial;
    } Memory;

    typedef struct {
        const std::wstring name;
        const std::wstring mac;
    } Network;

    typedef struct {
        const std::wstring letter;
        const std::wstring model;
        const std::wstring serial;
        const std::wstring interface_type;
        const std::wstring media_type;
        const unsigned long long capacity;
    } Disk;
}