/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: private: error.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
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
    {"invalid-arguments", "Invalid argument list"},
    {"invalid-thread", "Invalid thread entity"},
    {"invalid-entities", "Invalid entity list"},
    {"request-failed", "Failed to process specified request"},
    {"decode-failed", "Failed to decode specified data"},
    {"hash-mode-nonexistent", "Failed to fetch hash mode"},
    {"cipher-mode-nonexistent", "Failed to fetch cipher mode"},
    {"cipher-invalid-key", "Invalid cipher key"},
    {"cipher-invalid-iv", "Invalid cipher IV"},
    {"sound-invalid-loop", "Sound entity not in loopable state"},
    {"sound-invalid-3d", "Sound entity not in 3D state"},
    {"file-nonexistent", "Failed to fetch specified file"}
};