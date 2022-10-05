/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: private: error.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса
     DOC: 14/09/2022
     Desc: Error Codes
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/index.h>


/////////////////////////
// Vital: Error Codes //
/////////////////////////

std::map<std::string, std::string> Vital::ErrorCode = {
    {"request_failed", "Failed to process specified request"},
    {"decode-failed", "Failed to decode specified data"},
    {"hash-mode-nonexistent", "Invalid hash mode"},
    {"cipher-mode-nonexistent", "Invalid cipher mode"},
    {"cipher-invalid-key", "Invalid cipher key"},
    {"cipher-invalid-iv", "Invalid cipher IV"},
    {"invalid-3d-sound", "Invalid 3D sound"},
    {"file-nonexistent", "File non-existent"}
};