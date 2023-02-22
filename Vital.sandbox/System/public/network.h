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

    // Checkers //
    extern bool isConnected();

    // Setters //
    extern bool setPeerLimit(int limit);
    extern bool setBandwidthLimit(Vital::Type::Network::Bandwidth limit);

    // Getters //
    extern int getPeerLimit();
    extern Vital::Type::Network::Bandwidth getBandwidthLimit();

    // Utils //
    extern bool emit(const std::string& message, Vital::Type::Network::PeerID peer = 0);
}