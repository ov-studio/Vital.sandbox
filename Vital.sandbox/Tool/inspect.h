/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: public: inspect.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Inspect Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <Vital.sandbox/Tool/crypto.h>
#if defined(Vital_SDK_WINDOWS)
    #include <windows.h>
    #include <comdef.h>
    #include <wbemidl.h>
#else
    #include <sys/utsname.h>
#endif


///////////////////////////
// Vital: Tool: Inspect //
///////////////////////////

namespace Vital::Tool::Inspect {
    inline std::string normalize(std::string s) {
        s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); }), s.end());
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
        return s;
    }

    inline std::string hash(const std::string& s) {
        return Vital::System::Crypto::hash("SHA256", s);
    }

    #if defined(Vital_SDK_WINDOWS)
    class WMI {
        IWbemLocator* locator  = nullptr;
        IWbemServices* services = nullptr;
        public:
            explicit WMI(const wchar_t* root = L"ROOT\\CIMV2") {
                CoInitializeEx(nullptr, COINIT_MULTITHREADED);
                CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
                    RPC_C_AUTHN_LEVEL_DEFAULT,
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    nullptr, EOAC_NONE, nullptr);
                CoCreateInstance(
                    CLSID_WbemLocator, nullptr,
                    CLSCTX_INPROC_SERVER,
                    IID_PPV_ARGS(&locator));
                locator -> ConnectServer(
                    _bstr_t(root),
                    nullptr, nullptr, nullptr,
                    0, nullptr, nullptr, &services);
                CoSetProxyBlanket(
                    services,
                    RPC_C_AUTHN_WINNT,
                    RPC_C_AUTHZ_NONE,
                    nullptr,
                    RPC_C_AUTHN_LEVEL_CALL,
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    nullptr, EOAC_NONE);
            }

            ~WMI() {
                if (services) services -> Release();
                if (locator)  locator -> Release();
                CoUninitialize();
            }

            std::vector<std::wstring> query(const std::wstring& cls, const std::wstring& field) {
                std::vector<std::wstring> out;
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
                        if (v.vt == VT_BSTR && v.bstrVal)
                            out.emplace_back(v.bstrVal);
                    }
                    VariantClear(&v);
                    obj -> Release();
                }
                en -> Release();
                return out;
            }
    };
    #endif

    inline std::string cpu() {
        #if defined(Vital_SDK_WINDOWS)
        WMI wmi;
        auto ids = wmi.query(L"Win32_Processor", L"ProcessorId");
        if (!ids.empty()) return hash(normalize(std::string(ids[0].begin(), ids[0].end())));
        #else
        std::ifstream cpuinfo("/proc/cpuinfo");
        std::string line;
        while (std::getline(cpuinfo, line)) {
            if (line.rfind("Serial", 0) == 0 ||
                line.rfind("model name", 0) == 0)
                return hash(normalize(line));
        }
        #endif
        return {};
    }

    inline std::string system() {
        #if defined(Vital_SDK_WINDOWS)
        WMI wmi;
        auto vendor = wmi.query(L"Win32_BaseBoard", L"Manufacturer");
        auto product = wmi.query(L"Win32_BaseBoard", L"Product");
        std::string s;
        if (!vendor.empty())
            s += std::string(vendor[0].begin(), vendor[0].end());
        if (!product.empty())
            s += std::string(product[0].begin(), product[0].end());
        return s.empty() ? std::string{} : hash(normalize(s));
        #else
        struct utsname u{};
        uname(&u);
        return hash(normalize(
            std::string(u.sysname) +
            std::string(u.machine)));
        #endif
    }

    inline std::string disk() {
        #if defined(Vital_SDK_WINDOWS)
        WMI wmi;
        auto serials = wmi.query(L"Win32_DiskDrive", L"SerialNumber");
        if (!serials.empty())
            return hash(normalize(
                std::string(serials[0].begin(), serials[0].end())));
        #else
        for (auto& b : std::filesystem::directory_iterator("/sys/block")) {
            std::ifstream sn(b.path() / "serial");
            std::string serial;
            std::getline(sn, serial);
            if (!serial.empty())
                return hash(normalize(serial));
        }
        #endif
        return {};
    }

    inline std::string fingerprint() {
        std::string material;
        auto d = disk();
        auto c = cpu();
        auto s = system();
        if (!d.empty()) material += d;
        if (!c.empty()) material += c;
        if (!s.empty()) material += s;
        return hash(material);
    }
}