/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Type: public: network.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Type/public/index.h>


///////////////////////////
// Vital: Type: Network //
///////////////////////////

namespace Vital::Type::Network {
    typedef struct {
        const std::string host;
        const int port;
    } Address;

    typedef struct {
        int incoming;
        int outgoing;
    } Bandwidth;

    typedef unsigned long PeerID;
}