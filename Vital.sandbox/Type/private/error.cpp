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
    {"invalid-result", "Invalid result"},
    {"invalid-arguments", "Invalid argument list"},
    {"invalid-thread", "Invalid thread entity"},
    {"invalid-entities", "Invalid entity list"},
    {"request-failed", "Failed to process request ({})"},
    {"hash-mode-nonexistent", "Failed to fetch hash mode ({})"},
    {"cipher-mode-nonexistent", "Failed to fetch cipher mode ({})"},
    {"cipher-invalid-key", "Invalid cipher key"},
    {"cipher-invalid-iv", "Invalid cipher IV"},
    {"file-nonexistent", "Failed to fetch file ({})"},
    {"serial-nonexistent", "Failed to fetch device's serial"}
};