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
    const std::string parseMessage(enet_uint8* message, size_t size) { return std::string(reinterpret_cast<char*>(message), size); }

    bool isConnected() { return networkInstance ? true : false; }

    bool create(Vital::Type::Network::Address address) {
        if (isConnected()) return false;
        enet_initialize();
        ENetAddress networkAddress;
        enet_address_set_host(&networkAddress, address.host.c_str());
        networkAddress.port = static_cast<enet_uint16>(address.port);
        if (Vital::System::getPlatform() == "client") {
            networkInstance = enet_host_create(NULL, getPeerLimit(), 2, bandwidthLimit.incoming, bandwidthLimit.outgoing);
            networkPeer = enet_host_connect(networkInstance, &networkAddress, 2, 0);
        }
        else { networkInstance = enet_host_create(&networkAddress, getPeerLimit(), 2, bandwidthLimit.incoming, bandwidthLimit.outgoing); }
        return true;
    }

    bool destroy() {
        if (!isConnected()) return false;
        if (Vital::System::getPlatform() == "client") enet_peer_reset(networkPeer);
        enet_host_destroy(networkInstance);
        enet_deinitialize();
        networkInstance = nullptr;
        networkPeer = nullptr;
        networkPeers.clear();
        return true;
    }

    bool render() {
        if (!isConnected()) return false;
        ENetEvent networkEvent;
        enet_host_service(networkInstance, &networkEvent, 1000);
        if (Vital::System::getPlatform() == "client") {
            switch (networkEvent.type) {
                case ENET_EVENT_TYPE_RECEIVE: {
                    auto message = parseMessage(networkEvent.packet -> data, networkEvent.packet -> dataLength);
                    std::cout << "\n*[Server] " << message;
                    emit("Hello From Client");
                    enet_packet_destroy(networkEvent.packet);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    std::cout << "\n*Disconnected from server";
                    destroy();
                    break;
                }
            }
        }
        else {
            switch (networkEvent.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    networkEvent.peer -> data = reinterpret_cast<void*>(peerID);
                    networkPeers[peerID] = networkEvent.peer;
                    std::cout << "\n*Client connected [ID: " << getPeerID(networkEvent.peer) << "]";
                    emit("Hello From Server", getPeerID(networkEvent.peer));
                    peerID++;
                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE: {
                    auto message = parseMessage(networkEvent.packet -> data, networkEvent.packet -> dataLength);
                    std::cout << "\n*[Client: " << getPeerID(networkEvent.peer) << "] " << message;
                    enet_packet_destroy(networkEvent.packet);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    auto peerID = getPeerID(networkEvent.peer);
                    std::cout << "\n*Client disconnected [ID: " << getPeerID(networkEvent.peer) << "]";
                    networkPeers.erase(peerID);
                    networkEvent.peer -> data = NULL;
                    break;
                }
            }
        }
        return true;
    }

    bool setPeerLimit(int limit) {
        // TODO: ADD API TO GET PLATFORM TYPE
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

    bool emit(const std::string& buffer, Vital::Type::Network::PeerID peer) {
        if (!isConnected()) return false;
        if ((Vital::System::getPlatform() == "client") || (peer <= 0)) enet_host_broadcast(networkInstance, 0, enet_packet_create(buffer.c_str(), buffer.size() + 1, ENET_PACKET_FLAG_RELIABLE));
        else {
            if (!networkPeers[peer]) return false;
            enet_peer_send(networkPeers[peer], 0, enet_packet_create(buffer.c_str(), buffer.size() + 1, ENET_PACKET_FLAG_RELIABLE));
        }
        return true;
    }
}
