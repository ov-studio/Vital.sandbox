/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: network.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>


///////////////////////////
// Vital: Tool: Network //
///////////////////////////

namespace Vital::Tool::Network {
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