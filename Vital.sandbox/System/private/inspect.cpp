/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: private: inspect.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Inspect System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/inspect.h>
#include <windows.h>
#include <comdef.h>
#include <wbemidl.h>
#include <ntddscsi.h>


///////////////////////////
// Vital: Type: Inspect //
///////////////////////////

namespace Vital::System::Inspect {
    template <typename T = int>
    T toNumber(std::vector<T> value, int index = 0) {
        return ((index < 0) || (index > value.size())) ? 0 : value.at(index);
    }

    std::wstring toString(std::vector<const wchar_t*> value, int index = 0, bool clipWhitespaces = false) {
        auto result = std::wstring(((index < 0) || (index > value.size()) || (value.at(index) == nullptr)) ? L"" : value.at(index));
        if (clipWhitespaces) result.erase(std::remove(result.begin(), result.end(), L' '), result.end());
        result = result.length() <= 0 ? L"-" : result;
        return result;
    }

    std::wstring queryHKLM(const std::wstring& keyName, const std::wstring& fieldName) {
        DWORD querySize;
        std::wstring queryResult;
        RegGetValueW(HKEY_LOCAL_MACHINE, keyName.c_str(), fieldName.c_str(), RRF_RT_REG_SZ, nullptr, nullptr, &querySize);
        queryResult.resize(querySize);
        RegGetValueW(HKEY_LOCAL_MACHINE, keyName.c_str(), fieldName.c_str(), RRF_RT_REG_SZ, nullptr, &queryResult[0], &querySize);
        return toString({ queryResult.c_str() });
    }

    template <typename T = const wchar_t*>
    std::vector<T> queryWMI(const std::wstring& className, const std::wstring& fieldName, std::vector<T> queryResult = {}, const wchar_t* serverName = L"ROOT\\CIMV2") {
        int queryStage = 0;
        IWbemLocator* locator;
        IWbemServices* services;
        IEnumWbemClassObject* enumerator;
        HRESULT queryState = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (!FAILED(queryState)) {
            queryStage = 1;
            queryState = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
            if (!FAILED(queryState)) {
                queryState = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<PVOID*>(&locator));
                queryStage = 2;
                queryState = locator -> ConnectServer(_bstr_t(serverName), nullptr, nullptr, nullptr, NULL, nullptr, nullptr, &services);
                if (!FAILED(queryState)) {
                    queryStage = 3;
                    queryState = CoSetProxyBlanket(services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
                    if (!FAILED(queryState)) {
                        std::wstring queryString(L"SELECT ");
                        queryString.append(fieldName.c_str()).append(L" FROM ").append(className.c_str());
                        queryState = services -> ExecQuery(bstr_t(L"WQL"), bstr_t(queryString.c_str()), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &enumerator);
                        if (!FAILED(queryState)) {
                            queryStage = 4;
                            IWbemClassObject* queryObject;
                            VARIANT queryVariant {};
                            DWORD queryReturn;
                            while (enumerator) {
                                HRESULT result = enumerator -> Next(WBEM_INFINITE, 1, &queryObject, &queryReturn);
                                if (!queryReturn) break;
                                result = queryObject -> Get(fieldName.c_str(), 0, &queryVariant, nullptr, nullptr);
                                if (typeid(T) == typeid(long) || typeid(T) == typeid(int)) queryResult.push_back((T)queryVariant.intVal);
                                else if (typeid(T) == typeid(bool)) queryResult.push_back((T)queryVariant.boolVal);
                                else if (typeid(T) == typeid(unsigned int)) queryResult.push_back((T)queryVariant.uintVal);
                                else if (typeid(T) == typeid(unsigned short)) queryResult.push_back((T)queryVariant.uiVal);
                                else if (typeid(T) == typeid(long long)) queryResult.push_back((T)queryVariant.llVal);
                                else if (typeid(T) == typeid(unsigned long long)) queryResult.push_back((T)queryVariant.ullVal);
                                else queryResult.push_back((T)((bstr_t)queryVariant.bstrVal).copy());
                                VariantClear(&queryVariant);
                                queryObject -> Release();
                            }
                        }
                    }
                }
            }
        }
        if (FAILED(queryState)) queryResult.resize(0);
        if (queryStage > 3) enumerator -> Release();
        if (queryStage > 2) services -> Release();
        if (queryStage > 1) locator -> Release();
        if (queryStage > 0) CoUninitialize();
        return queryResult;
    }

    vSDK::Core::System::Type::System vSDK::Core::System::system() {
        auto name = toString(queryWMI(L"Win32_ComputerSystem", L"Name"));
        auto model = toString(queryWMI(L"Win32_ComputerSystem", L"Model"));
        auto manufacturer = toString(queryWMI(L"Win32_BaseBoard", L"Manufacturer"));
        auto hardware_id = queryHKLM(L"SYSTEM\\CurrentControlSet\\Control\\SystemInformation", L"ComputerHardwareId");
        auto os = toString(queryWMI(L"Win32_OperatingSystem", L"Name"));
        auto os_architecture = toString(queryWMI(L"Win32_OperatingSystem", L"OSArchitecture"));
        auto os_version = toString(queryWMI(L"Win32_OperatingSystem", L"Version"));
        auto os_serial = toString(queryWMI(L"Win32_OperatingSystem", L"SerialNumber"));
        if (os.find('|') != std::wstring::npos) os.resize(os.find('|'));
        return { name, model, manufacturer, hardware_id, os, os_architecture, os_version, os_serial };
    }

    vSDK::Core::System::Type::SMBIOS vSDK::Core::System::fetchSMBIOsInfo() {
        auto manufacturer = toString(queryWMI(L"Win32_BaseBoard", L"Manufacturer"));
        auto product = toString(queryWMI(L"Win32_BaseBoard", L"Product"));
        auto version = toString(queryWMI(L"Win32_BaseBoard", L"Version"));
        auto serial = toString(queryWMI(L"Win32_BaseBoard", L"SerialNumber"));
        return { manufacturer, product, version, serial };
    }

    vSDK::Core::System::Type::CPU vSDK::Core::System::fetchCPUInfo() {
        auto name = toString(queryWMI(L"Win32_Processor", L"Name"));
        auto manufacturer = toString(queryWMI(L"Win32_Processor", L"Manufacturer"));
        auto id = toString(queryWMI(L"Win32_Processor", L"ProcessorId"));
        auto cores = toNumber(queryWMI(L"Win32_Processor", L"NumberOfCores", std::vector<int> {}));
        auto threads = toNumber(queryWMI(L"Win32_Processor", L"NumberOfLogicalProcessors", std::vector<int> {}));
        return { name, manufacturer, id, cores, threads };
    }

    std::vector<vSDK::Core::System::Type::GPU> vSDK::Core::System::fetchGPUInfo() {
        auto name = queryWMI(L"Win32_VideoController", L"Name");
        auto version = queryWMI(L"Win32_VideoController", L"DriverVersion");
        auto refresh_rate = queryWMI(L"Win32_VideoController", L"CurrentRefreshRate", std::vector<int> {});
        std::pair<std::vector<int>, std::vector<int>> resolution = { queryWMI(L"Win32_VideoController", L"CurrentHorizontalResolution", std::vector<int> {}), queryWMI(L"Win32_VideoController", L"CurrentVerticalResolution", std::vector<int> {})};
        auto capacity = queryWMI(L"Win32_VideoController", L"AdapterRam", std::vector<unsigned long long> {});
        std::vector<vSDK::Core::System::Type::GPU> devices;
        devices.reserve(name.size());
        for (int i = 0; i < devices.capacity(); i++) {
            devices.push_back({ toString(name, i), toString(version, i), toNumber(refresh_rate, i), {toNumber(resolution.first, i), toNumber(resolution.second, i)}, static_cast<const unsigned long long>(toNumber(capacity, i)*2/pow(1024, 2)/1000) });
        }
        return devices;
    }

    std::vector<vSDK::Core::System::Type::Memory> vSDK::Core::System::fetchMemoryInfo() {
        auto manufacturer = queryWMI(L"Win32_PhysicalMemory", L"Manufacturer");
        auto version = queryWMI(L"Win32_PhysicalMemory", L"Version");
        auto serial = queryWMI(L"Win32_PhysicalMemory", L"SerialNumber");
        std::vector<vSDK::Core::System::Type::Memory> devices;
        devices.reserve(manufacturer.size());
        for (int i = 0; i < devices.capacity(); i++) {
            devices.push_back({ toString(manufacturer, i), toString(version, i), toString(serial, i) });
        }
        return devices;
    }

    std::vector<vSDK::Core::System::Type::Network> vSDK::Core::System::fetchNetworkInfo() {
        auto name = queryWMI(L"Win32_NetworkAdapter", L"Name");
        auto mac = queryWMI(L"Win32_NetworkAdapter", L"MACAddress");
        std::vector<vSDK::Core::System::Type::Network> devices;
        devices.reserve(name.size());
        for (int i = 0; i < devices.capacity(); i++) {
            devices.push_back({ toString(name, i), toString(mac, i) });
        }
        return devices;
    }

    std::vector<vSDK::Core::System::Type::Disk> vSDK::Core::System::disk() {
        int drives = 0;
        auto name = queryWMI(L"Win32_DiskDrive", L"Name");
        auto model = queryWMI(L"Win32_DiskDrive", L"Model");
        auto serial = queryWMI(L"Win32_DiskDrive", L"SerialNumber");
        auto interface_type = queryWMI(L"Win32_DiskDrive", L"InterfaceType");
        auto device_id = queryWMI(L"Win32_LogicalDisk", L"DeviceId");
        auto media_type = queryWMI(L"MSFT_PhysicalDisk", L"MediaType", std::vector<unsigned int> {}, L"ROOT\\microsoft\\windows\\storage");
        auto friendly_name = queryWMI(L"MSFT_PhysicalDisk", L"FriendlyName", {}, L"ROOT\\microsoft\\windows\\storage");
        HANDLE driveHandle;
        for (;; drives++) {
            if ((driveHandle = CreateFileW((L"\\\\.\\PhysicalDrive" + std::to_wstring(drives)).c_str(), NULL, NULL, nullptr, OPEN_EXISTING, NULL, nullptr)) == INVALID_HANDLE_VALUE) break;
            CloseHandle(driveHandle);
        }
        std::vector<const wchar_t*> device_id_sorted;
        device_id_sorted.reserve(drives);
        HANDLE volumeHandle;
        VOLUME_DISK_EXTENTS volumeDiskExtents;
        DWORD ioBytes;
        for (int i = 0; i < device_id_sorted.capacity(); i++) {
            for (int j = 0; j < device_id_sorted.capacity(); j++) {
                volumeHandle = CreateFileW((L"\\\\.\\" + toString(device_id, j)).c_str(), NULL, NULL, nullptr, OPEN_EXISTING, NULL, nullptr);
                DeviceIoControl(volumeHandle, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, nullptr, NULL, &volumeDiskExtents, sizeof(volumeDiskExtents), &ioBytes, nullptr);
                DeviceIoControl(volumeHandle, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, nullptr, NULL, &volumeDiskExtents, offsetof(VOLUME_DISK_EXTENTS, Extents[volumeDiskExtents.NumberOfDiskExtents]), &ioBytes, nullptr);
                CloseHandle(volumeHandle);
                if (volumeDiskExtents.Extents -> DiskNumber == std::stoi(&toString(name, i).back())) {
                    device_id_sorted.push_back(device_id.at(j));
                    break;
                }
            }
        }
        std::vector<vSDK::Core::System::Type::Disk> devices;
        devices.reserve(device_id_sorted.capacity());
        ULARGE_INTEGER diskCapacity;
        for (int i = 0; i < devices.capacity(); i++) {
            std::wstring media_typename = L"-";
            for (int j = 0; j < devices.capacity(); j++) {
                if (!toString(model, i).compare(friendly_name.at(j))) media_type.at(i) = media_type.at(j);
            }
            switch(media_type.at(i)) {
                case 3:
                    media_typename = L"HDD";
                    break;
                case 4:
                    media_typename = L"SSD";
                    break;
                case 5:
                    media_typename = L"SCM";
                    break;
            }
            GetDiskFreeSpaceEx(toString(device_id_sorted, i).c_str(), nullptr, &diskCapacity, nullptr);
            devices.push_back({ toString(device_id_sorted, i), toString(model, i), toString(serial, i, true), toString(interface_type, i), media_typename, static_cast<const unsigned long long>(diskCapacity.QuadPart/pow(1024, 3)) });
        }
        return devices;
    }
}
