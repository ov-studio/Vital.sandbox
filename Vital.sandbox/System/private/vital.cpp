/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: public: vital.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Vital System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/inspect.h>
#include <System/public/crypto.h>


////////////////////
// Vital: System //
////////////////////

// TODO: REMOVE LATER
#define vSDK_Client true

namespace Vital::System {
    std::string getPlatform() {
        #if defined(vSDK_Client) && defined(WINDOWS) 
            return "client";
        #else
            return "server";
        #endif
    }

    std::string systemSerial = "";
    std::string getSystemSerial() {
        if (systemSerial.empty() && (getPlatform() == "client")) {
            auto system = Vital::System::Inspect::system();
            auto smbios = Vital::System::Inspect::smbios();
            auto cpu = Vital::System::Inspect::cpu();
            auto gpu = Vital::System::Inspect::gpu();
            auto memory = Vital::System::Inspect::memory();
            auto network = Vital::System::Inspect::network();
            auto disk = Vital::System::Inspect::disk();
            systemSerial = "[";
            systemSerial += "\n\tSystem: " + Vital::System::Crypto::hash("SHA256", std::string(system.hardware_id.begin(), system.hardware_id.end())) + ",";
            systemSerial += "\n\tOS: " + Vital::System::Crypto::hash("SHA256", std::string(system.os_serial.begin(), system.os_serial.end())) + ",";
            systemSerial += "\n\tSMBIOS: " + Vital::System::Crypto::hash("SHA256", std::string(smbios.serial.begin(), smbios.serial.end())) + ",";
            systemSerial += "\n\tCPU: " + Vital::System::Crypto::hash("SHA256", std::string(cpu.id.begin(), cpu.id.end())) + ",";
            for (int i = 0; i < memory.size(); i++) {
                systemSerial += "\n\tMemory: " + Vital::System::Crypto::hash("SHA256", std::string(memory.at(i).serial.begin(), memory.at(i).serial.end())) + ",";
            }
            for (int i = 0; i < network.size(); i++) {
                if (network.at(i).mac != L"-") systemSerial += "\n\tNetwork: " + Vital::System::Crypto::hash("SHA256", std::string(network.at(i).mac.begin(), network.at(i).mac.end())) + ",";
            }
            for (int i = 0; i < disk.size(); i++) {
                systemSerial += "\n\tDisk: " + Vital::System::Crypto::hash("SHA256", std::string(disk.at(i).serial.begin(), disk.at(i).serial.end())) + ",";
            }
            systemSerial = systemSerial.substr(0, systemSerial.size() - 1);
            systemSerial += "\n]";
        }
        if (systemSerial.empty()) throw ErrorCode["serial-nonexistent"];
        return systemSerial;
    }

    unsigned int getSystemTick() {
        return static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()/1000000);
    }

    unsigned int applicationTick;
    unsigned int getApplicationTick() {
        applicationTick = applicationTick ? applicationTick : getSystemTick();
        return getSystemTick() - applicationTick;
    }

    unsigned int clientTick;
    unsigned int getClientTick() {
        clientTick = clientTick ? clientTick : getApplicationTick();
        return getSystemTick() - clientTick;
    }

    bool resetClientTick() {
        clientTick = getApplicationTick();
        return true;
    }
}