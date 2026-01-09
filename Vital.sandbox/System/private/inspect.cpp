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
#include <winioctl.h>
#include <comdef.h>
#include <wbemidl.h>
#include <ntddscsi.h>


///////////////////////////
// Vital: Type: Inspect //
///////////////////////////

namespace Vital::System::Inspect {
    template<typename T>
    inline T toNumber(const std::vector<T>& v, size_t i = 0) noexcept {
        return (i < v.size()) ? v[i] : T{};
    }

    inline std::wstring toString(const std::vector<std::wstring>& v, size_t i = 0, bool trim_spaces = false) {
        if (i >= v.size() || v[i].empty()) return L"-";
        std::wstring s = v[i];
        if (trim_spaces) {
            s.erase(std::remove_if(s.begin(), s.end(), iswspace), s.end());
        }
        return s;
    }

    inline std::wstring queryHKLM(const std::wstring& key, const std::wstring& value) {
        DWORD size = 0;
        if (RegGetValueW(HKEY_LOCAL_MACHINE, key.c_str(), value.c_str(), RRF_RT_REG_SZ, nullptr, nullptr, &size) != ERROR_SUCCESS) return L"-";
        std::wstring out(size / sizeof(wchar_t), L'\0');
        if (RegGetValueW(HKEY_LOCAL_MACHINE, key.c_str(), value.c_str(), RRF_RT_REG_SZ, nullptr, out.data(), &size) != ERROR_SUCCESS) return L"-";
        out.resize(wcslen(out.c_str()));
        return out;
    }

    class WMI {
        private:
            IWbemLocator*  locator  = nullptr;
            IWbemServices* services = nullptr;
        public:
            explicit WMI(const wchar_t* root = L"ROOT\\CIMV2") {
                CoInitializeEx(nullptr, COINIT_MULTITHREADED);
                CoInitializeSecurity(
                    nullptr, -1, nullptr, nullptr,
                    RPC_C_AUTHN_LEVEL_DEFAULT,
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    nullptr, EOAC_NONE, nullptr
                );
                CoCreateInstance(
                    CLSID_WbemLocator, nullptr,
                    CLSCTX_INPROC_SERVER,
                    IID_PPV_ARGS(&locator)
                );
                locator -> ConnectServer(
                    _bstr_t(root),
                    nullptr, nullptr, nullptr,
                    0, nullptr, nullptr,
                    &services
                );
                CoSetProxyBlanket(
                    services,
                    RPC_C_AUTHN_WINNT,
                    RPC_C_AUTHZ_NONE,
                    nullptr,
                    RPC_C_AUTHN_LEVEL_CALL,
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    nullptr,
                    EOAC_NONE
                );
            }
            ~WMI() {
                if (services) services -> Release();
                if (locator) locator -> Release();
                CoUninitialize();
            }
            template<typename T>
            std::vector<T> query(const std::wstring& cls, const std::wstring& field);
    };

    template<typename T>
    std::vector<T> WMI::query(const std::wstring& cls, const std::wstring& field) {
        std::vector<T> out;
        IEnumWbemClassObject* en = nullptr;
        std::wstring q = L"SELECT " + field + L" FROM " + cls;
        if (FAILED(services -> ExecQuery(
            bstr_t(L"WQL"),
            bstr_t(q.c_str()),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr,
            &en))
        ) return out;
        IWbemClassObject* obj = nullptr;
        ULONG ret = 0;
        while (en -> Next(WBEM_INFINITE, 1, &obj, &ret) == S_OK) {
            VARIANT v;
            VariantInit(&v);
            if (SUCCEEDED(obj -> Get(field.c_str(), 0, &v, nullptr, nullptr))) {
                if constexpr (std::is_same_v<T, std::wstring>) out.emplace_back(v.bstrVal ? v.bstrVal : L"-");
                else if constexpr (std::is_integral_v<T>) out.emplace_back(static_cast<T>(v.ullVal));
            }
            VariantClear(&v);
            obj -> Release();
        }
        en -> Release();
        return out;
    }

    Vital::Type::Inspect::System system() {
        WMI wmi;
        auto os = toString(wmi.query<std::wstring>(L"Win32_OperatingSystem", L"Name"));
        if (auto p = os.find(L'|'); p != std::wstring::npos) os.resize(p);
        return {
            toString(wmi.query<std::wstring>(L"Win32_ComputerSystem", L"Name")),
            toString(wmi.query<std::wstring>(L"Win32_ComputerSystem", L"Model")),
            toString(wmi.query<std::wstring>(L"Win32_BaseBoard", L"Manufacturer")),
            queryHKLM(L"SYSTEM\\CurrentControlSet\\Control\\SystemInformation", L"ComputerHardwareId"),
            os,
            toString(wmi.query<std::wstring>(L"Win32_OperatingSystem", L"OSArchitecture")),
            toString(wmi.query<std::wstring>(L"Win32_OperatingSystem", L"Version")),
            toString(wmi.query<std::wstring>(L"Win32_OperatingSystem", L"SerialNumber"))
        };
    }

    Vital::Type::Inspect::SMBIOS smbios() {
        WMI wmi;
        return {
            toString(wmi.query<std::wstring>(L"Win32_BaseBoard", L"Manufacturer")),
            toString(wmi.query<std::wstring>(L"Win32_BaseBoard", L"Product")),
            toString(wmi.query<std::wstring>(L"Win32_BaseBoard", L"Version")),
            toString(wmi.query<std::wstring>(L"Win32_BaseBoard", L"SerialNumber"))
        };
    }

    Vital::Type::Inspect::CPU cpu() {
        WMI wmi;
        const uint32_t cores_u = toNumber(wmi.query<uint32_t>(L"Win32_Processor", L"NumberOfCores"));
        const uint32_t threads_u = toNumber(wmi.query<uint32_t>(L"Win32_Processor", L"NumberOfLogicalProcessors"));
        return {
            toString(wmi.query<std::wstring>(L"Win32_Processor", L"Name")),
            toString(wmi.query<std::wstring>(L"Win32_Processor", L"Manufacturer")),
            toString(wmi.query<std::wstring>(L"Win32_Processor", L"ProcessorId")),
            static_cast<int>(cores_u),
            static_cast<int>(threads_u)
        };
    }

    std::vector<Vital::Type::Inspect::GPU> gpu() {
        WMI wmi;
        auto names   = wmi.query<std::wstring>(L"Win32_VideoController", L"Name");
        auto drivers = wmi.query<std::wstring>(L"Win32_VideoController", L"DriverVersion");
        auto vram    = wmi.query<uint64_t>(L"Win32_VideoController", L"AdapterRAM");
        std::vector<Vital::Type::Inspect::GPU> out;
        out.reserve(names.size());
        for (size_t i = 0; i < names.size(); ++i) {
            out.push_back({
                toString(names, i),
                toString(drivers, i),
                0,
                {0, 0},
                static_cast<unsigned long long>((i < vram.size()) ? (vram[i] >> 30) : 0)
            });
        }
        return out;
    }

    std::vector<Vital::Type::Inspect::Memory> memory() {
        WMI wmi;
        auto m = wmi.query<std::wstring>(L"Win32_PhysicalMemory", L"Manufacturer");
        auto v = wmi.query<std::wstring>(L"Win32_PhysicalMemory", L"Version");
        auto s = wmi.query<std::wstring>(L"Win32_PhysicalMemory", L"SerialNumber");
        std::vector<Vital::Type::Inspect::Memory> out;
        out.reserve(m.size());
        for (size_t i = 0; i < m.size(); ++i) {
            out.push_back({
                toString(m, i),
                toString(v, i),
                toString(s, i)
            });
        }
        return out;
    }

    std::vector<Vital::Type::Inspect::Network> network() {
        WMI wmi;
        auto n = wmi.query<std::wstring>(L"Win32_NetworkAdapter", L"Name");
        auto m = wmi.query<std::wstring>(L"Win32_NetworkAdapter", L"MACAddress");
        std::vector<Vital::Type::Inspect::Network> out;
        out.reserve(n.size());
        for (size_t i = 0; i < n.size(); ++i) {
            out.push_back({
                toString(n, i),
                toString(m, i)
            });
        }
        return out;
    }

    std::vector<Vital::Type::Inspect::Disk> disk() {
        WMI wmi;
        auto device_id = wmi.query<std::wstring>(L"Win32_DiskDrive", L"DeviceID");
        auto model = wmi.query<std::wstring>(L"Win32_DiskDrive", L"Model");
        auto serial = wmi.query<std::wstring>(L"Win32_DiskDrive", L"SerialNumber");
        auto interface_type = wmi.query<std::wstring>(L"Win32_DiskDrive", L"InterfaceType");
        auto size_bytes = wmi.query<uint64_t>(L"Win32_DiskDrive", L"Size");
        std::vector<Vital::Type::Inspect::Disk> out;
        out.reserve(device_id.size());
        for (size_t i = 0; i < device_id.size(); ++i) {
            out.push_back({
                toString(device_id, i),
                toString(model, i),
                toString(serial, i, true),
                toString(interface_type, i),
                L"-",
                static_cast<unsigned long long>((i < size_bytes.size()) ? (size_bytes[i] >> 30) : 0)
            });
        }
        return out;
    }    
}
