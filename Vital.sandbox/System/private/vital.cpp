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

#if defined(WINDOWS)
    #include <Windows.h>
#endif


////////////////////
// Vital: System //
////////////////////

namespace Vital::System {
    std::string vsdk_serial = "";
    unsigned int vsdk_apptick;
    unsigned int vsdk_clienttick;

    std::string getSystemPlatform() { 
        #ifdef Vital_SDK_Client
            return "client";
        #else
            return "server";
        #endif
    }

    std::string getSystemSerial() {
        if (vsdk_serial.empty() && (getSystemPlatform() == "client")) {
            auto system = Vital::System::Inspect::system();
            auto smbios = Vital::System::Inspect::smbios();
            auto cpu = Vital::System::Inspect::cpu();
            auto gpu = Vital::System::Inspect::gpu();
            auto memory = Vital::System::Inspect::memory();
            auto network = Vital::System::Inspect::network();
            auto disk = Vital::System::Inspect::disk();
            vsdk_serial += "\"System\": \"" + Vital::System::Crypto::hash("SHA256", std::string(system.hardware_id.begin(), system.hardware_id.end())) + "\"";
            vsdk_serial += "\n\"OS\": \"" + Vital::System::Crypto::hash("SHA256", std::string(system.os_serial.begin(), system.os_serial.end())) + "\"";
            vsdk_serial += "\n\"SMBIOS\": \"" + Vital::System::Crypto::hash("SHA256", std::string(smbios.serial.begin(), smbios.serial.end())) + "\"";
            vsdk_serial += "\n\"CPU\": \"" + Vital::System::Crypto::hash("SHA256", std::string(cpu.id.begin(), cpu.id.end())) + "\"";
            for (int i = 0; i < memory.size(); i++) {
                vsdk_serial += "\n\"Memory-" + std::to_string(i + 1) + "\": \"" + Vital::System::Crypto::hash("SHA256", std::string(memory.at(i).serial.begin(), memory.at(i).serial.end())) + "\"";
            }
            for (int i = 0; i < network.size(); i++) {
                if (network.at(i).mac != L"-") vsdk_serial += "\n\"Network\": \"" + Vital::System::Crypto::hash("SHA256", std::string(network.at(i).mac.begin(), network.at(i).mac.end())) + "\"";
            }
            for (int i = 0; i < disk.size(); i++) {
                vsdk_serial += "\n\"Disk\": \"" + Vital::System::Crypto::hash("SHA256", std::string(disk.at(i).serial.begin(), disk.at(i).serial.end())) + "\"";
            }
        }
        if (vsdk_serial.empty()) throw std::runtime_error(ErrorCode["serial-nonexistent"]);
        return vsdk_serial;
    }

    unsigned int getSystemTick() {
        return static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()/1000000);
    }

    unsigned int getAppTick() {
        vsdk_apptick = vsdk_apptick ? vsdk_apptick : getSystemTick();
        return getSystemTick() - vsdk_apptick;
    }

    unsigned int getClientTick() {
        vsdk_clienttick = vsdk_clienttick ? vsdk_clienttick : getAppTick();
        return getSystemTick() - vsdk_clienttick;
    }

    bool resetClientTick() {
        vsdk_clienttick = getAppTick();
        return true;
    }
}