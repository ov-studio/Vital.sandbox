/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: network.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>


///////////////////////////
// Vital: Tool: Network //
///////////////////////////

namespace Vital::System::Network {
    // Managers //
    extern bool start(Vital::Tool::Network::Address address);
    extern bool stop();
    extern bool update();

    // APIs //
    extern bool isConnected();
    extern bool setPeerLimit(int limit);
    extern int getPeerLimit();
    extern bool setBandwidthLimit(Vital::Tool::Network::Bandwidth limit);
    extern Vital::Tool::Network::Bandwidth getBandwidthLimit();

    // Utils //
    extern bool emit(Vital::Tool::Stack buffer, Vital::Tool::Network::PeerID peerID = 0, bool isLatent = false);
}