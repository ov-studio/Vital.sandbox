/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: private: network.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Network System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/network.h>
#include <System/public/crypto.h>
#include <System/public/event.h>
#include <Vendor/enet/enet.h>


///////////////////////////
// Vital: Type: Network //
///////////////////////////

namespace Vital::System::Network {
    ENetHost* networkInstance = nullptr;
    ENetPeer* networkPeer = nullptr;
    Vital::Type::Network::Bandwidth bandwidthLimit = {0, 0};
    Vital::Type::Network::PeerID peerID = 1; int peerLimit = 32;
    std::map<Vital::Type::Network::PeerID, ENetPeer*> networkPeers;
    Vital::Type::Network::PeerID getPeerID(ENetPeer* peer) { return reinterpret_cast<Vital::Type::Network::PeerID>(peer -> data); }

    // Managers //
    bool start(Vital::Type::Network::Address address) {
        if (networkInstance) return false;
        enet_initialize();
        ENetAddress networkAddress;
        enet_address_set_host(&networkAddress, address.host.c_str());
        networkAddress.port = static_cast<enet_uint16>(address.port);
        auto bandwidthLimit = getBandwidthLimit();
        if (Vital::System::getPlatform() == "client") {
            networkInstance = enet_host_create(NULL, getPeerLimit(), 2, bandwidthLimit.incoming, bandwidthLimit.outgoing);
            networkPeer = enet_host_connect(networkInstance, &networkAddress, 2, 0);
        }
        else { networkInstance = enet_host_create(&networkAddress, getPeerLimit(), 2, bandwidthLimit.incoming, bandwidthLimit.outgoing); }
        return true;
    }
    bool stop() {
        if (!networkInstance) return false;
        if (Vital::System::getPlatform() == "client") enet_peer_reset(networkPeer);
        enet_host_destroy(networkInstance);
        enet_deinitialize();
        networkInstance = nullptr;
        networkPeer = nullptr;
        networkPeers.clear();
        return true;
    }
    bool update() {
        if (!networkInstance) return false;
        ENetEvent networkEvent;
        enet_host_service(networkInstance, &networkEvent, 1000);
        switch (networkEvent.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                if (Vital::System::getPlatform() == "server") {
                    networkEvent.peer -> data = reinterpret_cast<void*>(peerID);
                    networkPeers.emplace(peerID, networkEvent.peer);
                    Vital::Type::Stack::Instance eventArguments;
                    eventArguments.push("peerID", getPeerID(networkEvent.peer));
                    Vital::System::Event::emit("Network:@PeerConnection", eventArguments);
                    peerID++;
                }
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE: {
                Vital::Type::Stack::Instance eventArguments = Vital::Type::Stack::Instance::deserialize(Vital::System::Crypto::decode(std::string(reinterpret_cast<char*>(networkEvent.packet -> data), networkEvent.packet -> dataLength)));
                if (Vital::System::getPlatform() == "server") eventArguments.push("peerID", getPeerID(networkEvent.peer));
                Vital::System::Event::emit("Network:@PeerMessage", eventArguments);
                enet_packet_destroy(networkEvent.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT: {
                Vital::Type::Stack::Instance eventArguments;
                if (Vital::System::getPlatform() == "server") eventArguments.push("peerID", getPeerID(networkEvent.peer));
                Vital::System::Event::emit("Network:@PeerDisconnection", eventArguments);
                if (Vital::System::getPlatform() == "client") stop();
                else {
                    networkPeers.erase(getPeerID(networkEvent.peer));
                    networkEvent.peer -> data = NULL;
                }
                break;
            }
        }
        return true;
    }

    // APIs //
    bool isConnected() { return networkInstance ? true : false; }
    bool setPeerLimit(int limit) {
        if (Vital::System::getPlatform() != "server") return false;
        peerLimit = limit;
        return true;
    }
    int getPeerLimit() { return Vital::System::getPlatform() == "server" ? peerLimit : 1; }
    bool setBandwidthLimit(Vital::Type::Network::Bandwidth limit) {
        bandwidthLimit = limit;
        return true;
    }
    Vital::Type::Network::Bandwidth getBandwidthLimit() { return bandwidthLimit; }

    // Utils //
    bool emit(Vital::Type::Stack::Instance buffer, Vital::Type::Network::PeerID peer, bool isLatent) {
        // TODO: ADD LATENT MODE
        if (!isConnected()) return false;
        const std::string bufferSerial = Vital::System::Crypto::encode(buffer.serialize());
        if ((Vital::System::getPlatform() == "client") || (peer <= 0)) {
            if (!isLatent) enet_host_broadcast(networkInstance, 0, enet_packet_create(bufferSerial.c_str(), bufferSerial.size(), ENET_PACKET_FLAG_RELIABLE));
        }
        else {
            if (!networkPeers.at(peer)) return false;
            if (!isLatent) enet_peer_send(networkPeers.at(peer), 0, enet_packet_create(bufferSerial.c_str(), bufferSerial.size(), ENET_PACKET_FLAG_RELIABLE));
        }
        return true;
    }
}