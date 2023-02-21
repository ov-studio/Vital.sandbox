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
    extern bool isConnected();
    extern bool create(Vital::Type::Network::Address address);
    extern bool destroy();
    extern bool update();
    extern bool setPeerLimit(int limit);
    extern int getPeerLimit();
    extern bool setBandwidthLimit(Vital::Type::Network::Bandwidth limit);
    extern bool emit(const std::string& buffer, Vital::Type::Network::PeerID peer = 0);
    extern Vital::Type::Network::Bandwidth getBandwidthLimit();
}