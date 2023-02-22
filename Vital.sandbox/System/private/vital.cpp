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

namespace Vital::System {
    std::string systemPlatform = "server";
    bool setPlatform(const std::string& platform) {
        if (platform == "client") {
            #if !defined(WINDOWS)
                return false;
            #endif
        }
        else if (platform != "server") return false;
        systemPlatform = platform;
        return true;
    }
    std::string getPlatform() { return systemPlatform; }

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
            systemSerial += "\n\t\"System: " + Vital::System::Crypto::hash("SHA256", std::string(system.hardware_id.begin(), system.hardware_id.end())) + "\",";
            systemSerial += "\n\t\"OS: " + Vital::System::Crypto::hash("SHA256", std::string(system.os_serial.begin(), system.os_serial.end())) + "\",";
            systemSerial += "\n\t\"SMBIOS: " + Vital::System::Crypto::hash("SHA256", std::string(smbios.serial.begin(), smbios.serial.end())) + "\",";
            systemSerial += "\n\t\"CPU: " + Vital::System::Crypto::hash("SHA256", std::string(cpu.id.begin(), cpu.id.end())) + "\",";
            for (int i = 0; i < memory.size(); i++) {
                systemSerial += "\n\t\"Memory: " + Vital::System::Crypto::hash("SHA256", std::string(memory.at(i).serial.begin(), memory.at(i).serial.end())) + "\",";
            }
            for (int i = 0; i < network.size(); i++) {
                if (network.at(i).mac != L"-") systemSerial += "\n\t\"Network: " + Vital::System::Crypto::hash("SHA256", std::string(network.at(i).mac.begin(), network.at(i).mac.end())) + "\",";
            }
            for (int i = 0; i < disk.size(); i++) {
                systemSerial += "\n\t\"Disk: " + Vital::System::Crypto::hash("SHA256", std::string(disk.at(i).serial.begin(), disk.at(i).serial.end())) + "\",";
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