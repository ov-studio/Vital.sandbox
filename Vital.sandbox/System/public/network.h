/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: network.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/vital.h>


///////////////////////////
// Vital: Type: Network //
///////////////////////////

namespace Vital::System::Network {
    // Managers //
    extern bool start(Vital::Type::Network::Address address);
    extern bool stop();
    extern bool update();

    // APIs //
    extern bool isConnected();
    extern bool setPeerLimit(int limit);
    extern int getPeerLimit();
    extern bool setBandwidthLimit(Vital::Type::Network::Bandwidth limit);
    extern Vital::Type::Network::Bandwidth getBandwidthLimit();

    // Utils //
    extern bool emit(Vital::Type::Stack buffer, Vital::Type::Network::PeerID peerID = 0, bool isLatent = false);
}