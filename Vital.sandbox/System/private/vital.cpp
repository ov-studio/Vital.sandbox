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
    std::wstring systemSerial = L"-";
    unsigned int applicationTick, clientTick;

    std::string getPlatform() {
        #if defined(vSDK_Client) && defined(WINDOWS) 
            return "client";
        #else
            return "server";
        #endif
    }

    std::wstring getSystemSerial() {
        if (systemSerial.empty() && (getPlatform() == "client")) {
            auto system = Vital::System::Inspect::system();
            auto smbios = Vital::System::Inspect::smbios();
            auto cpu = Vital::System::Inspect::cpu();
            auto gpu = Vital::System::Inspect::gpu();
            auto memory = Vital::System::Inspect::memory();
            auto network = Vital::System::Inspect::network();
            auto disk = Vital::System::Inspect::disk();
            systemSerial = L"[";
            systemSerial += L"\n\tSystem: " + system.hardware_id + L",";
            systemSerial += L"\n\tOS: " + system.os_serial + L",";
            systemSerial += L"\n\tSMBIOS: " + smbios.serial + L",";
            systemSerial += L"\n\tCPU: " + cpu.id + L",";
            for (int i = 0; i < memory.size(); i++) {
                systemSerial += L"\n\tMemory: " + memory.at(i).serial + L",";
            }
            for (int i = 0; i < network.size(); i++) {
                if (network.at(i).mac != L"-") systemSerial += L"\n\tNetwork: " + network.at(i).mac + L",";
            }
            for (int i = 0; i < disk.size(); i++) {
                systemSerial += L"\n\tDisk: " + disk.at(i).serial + L",";
            }
            // TODO: TRIM LAST `,`
            systemSerial += L"\n]";
        }
        return systemSerial;
    }

    unsigned int getSystemTick() {
        return static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()/1000000);
    }

    unsigned int getApplicationTick() {
        applicationTick = applicationTick ? applicationTick : getSystemTick();
        return getSystemTick() - applicationTick;
    }

    unsigned int getClientTick() {
        clientTick = clientTick ? clientTick : getApplicationTick();
        return getSystemTick() - clientTick;
    }

    bool resetClientTick() {
        clientTick = getApplicationTick();
        return true;
    }
}