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
    ENetHost* network_instance = nullptr;
    ENetPeer* network_peer = nullptr;
    std::map <Vital::Type::Network::PeerID, ENetPeer*> network_peers;
    Vital::Type::Network::PeerID peer_id = 1;
    Vital::Type::Network::PeerID getPeerID(ENetPeer* peer) { return reinterpret_cast<Vital::Type::Network::PeerID>(peer -> data); }
    const std::string parseMessage(enet_uint8* message, size_t size) { return std::string(reinterpret_cast<char*>(message), size); }

    bool isConnected() { return network_instance ? true : false; }
    bool create(Vital::Type::Network::Address address) {
        if (isConnected()) return false;
        enet_initialize();
        ENetAddress network_address;
        enet_address_set_host(&network_address, address.host.c_str());
        network_address.port = static_cast<enet_uint16>(address.port);
        auto bandwidthLimit = getBandwidthLimit();
        if (vSDK::Core::getPlatformType() == "client") {
            network_instance = enet_host_create(NULL, getPeerLimit(), 2, bandwidthLimit.incoming, bandwidthLimit.outgoing);
            network_peer = enet_host_connect(network_instance, &network_address, 2, 0);
        }
        else { network_instance = enet_host_create(&network_address, getPeerLimit(), 2, bandwidthLimit.incoming, bandwidthLimit.outgoing); }
        return true;
    }
    bool destroy() {
        if (!isConnected()) return false;
        if (vSDK::Core::getPlatformType() == "client") enet_peer_reset(network_peer);
        enet_host_destroy(network_instance);
        enet_deinitialize();
        network_instance = nullptr;
        network_peer = nullptr;
        network_peers.clear();
        return true;
    }

    bool render() {
        if (!isConnected()) return false;
        ENetEvent network_event;
        enet_host_service(network_instance, &network_event, 1000);
        if (vSDK::Core::getPlatformType() == "client") {
            switch (network_event.type) {
                case ENET_EVENT_TYPE_RECEIVE: {
                    auto message = parseMessage(network_event.packet -> data, network_event.packet -> dataLength);
                    std::cout << "\n*[Server] " << message;
                    emit("Hello From Client");
                    enet_packet_destroy(network_event.packet);
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
            switch (network_event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    network_event.peer -> data = reinterpret_cast<void*>(peer_id);
                    network_peers[peer_id] = network_event.peer;
                    std::cout << "\n*Client connected [ID: " << getPeerID(network_event.peer) << "]";
                    emit("Hello From Server", getPeerID(network_event.peer));
                    peer_id++;
                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE: {
                    auto message = parseMessage(network_event.packet -> data, network_event.packet -> dataLength);
                    std::cout << "\n*[Client: " << getPeerID(network_event.peer) << "] " << message;
                    enet_packet_destroy(network_event.packet);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    auto peerID = getPeerID(network_event.peer);
                    std::cout << "\n*Client disconnected [ID: " << getPeerID(network_event.peer) << "]";
                    network_peers.erase(peerID);
                    network_event.peer -> data = NULL;
                    break;
                }
            }
        }
        return true;
    }

    int peer_limit = 32;
    bool setPeerLimit(int limit) {
        if (vSDK::Core::getPlatformType() != "server") return false;
        peer_limit = limit;
        return true;
    }
    int getPeerLimit() { return vSDK::Core::getPlatformType() == "server" ? peer_limit : 1; }

    Vital::Type::Network::Bandwidth bandwidth_limit = {0, 0};
    bool setBandwidthLimit(Vital::Type::Network::Bandwidth limit) {
        bandwidth_limit = limit;
        return true;
    }
    Vital::Type::Network::Bandwidth getBandwidthLimit() { return bandwidth_limit; }

    bool emit(const std::string& buffer, Vital::Type::Network::PeerID peer) {
        if (!isConnected()) return false;
        if ((vSDK::Core::getPlatformType() == "client") || (peer <= 0)) enet_host_broadcast(network_instance, 0, enet_packet_create(buffer.c_str(), buffer.size() + 1, ENET_PACKET_FLAG_RELIABLE));
        else {
            if (!network_peers[peer]) return false;
            enet_peer_send(network_peers[peer], 0, enet_packet_create(buffer.c_str(), buffer.size() + 1, ENET_PACKET_FLAG_RELIABLE));
        }
        return true;
    }
}
