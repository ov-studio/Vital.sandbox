/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: localserver.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 08/10/2026
     Desc: Local Server Registry
 ----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>


///////////////////////////////
// Vital: Tool: LocalServer //
///////////////////////////////

// Servers running on this machine announce themselves by writing a tiny JSON file
// (one per network port) into a shared temp directory; the client's main menu lists
// that directory and verifies each entry against the server's /info endpoint.
// Any port works: nothing is hard-coded, and stale files of crashed servers are
// simply ignored because their /info never answers.
namespace Vital::Tool::LocalServer {
    struct Entry {
        int port = 0;
        int http_port = 0;
    };

    #if !defined(VSDK_Client)
    namespace Internal {
        inline std::mutex mutex;
        inline std::set<int> ports;
    }
    #endif

    inline std::filesystem::path get_directory() {
        std::error_code ec;
        return std::filesystem::temp_directory_path(ec) / "vital.sandbox" / "servers";
    }

    inline std::filesystem::path get_path(int port) {
        return get_directory() / (std::to_string(port) + ".json");
    }

    #if !defined(VSDK_Client)
    inline void retract_published(int port) {
        std::lock_guard<std::mutex> lock(Internal::mutex);
        std::error_code ec;
        std::filesystem::remove(get_path(port), ec);
        Internal::ports.erase(port);
    }

    inline void retract_all() {
        std::lock_guard<std::mutex> lock(Internal::mutex);
        std::error_code ec;
        for (int port : Internal::ports) std::filesystem::remove(get_path(port), ec);
        Internal::ports.clear();
    }

    inline bool publish(int port, int http_port) {
        std::error_code ec;
        std::filesystem::create_directories(get_directory(), ec);
        if (ec) return false;
        std::ofstream file(get_path(port));
        file << "{\"port\":" << port << ",\"http_port\":" << http_port << "}";
        file.close();
        if (file.fail()) return false;
        static const bool hooked = (std::atexit(retract_all), true);
        (void)hooked;
        std::lock_guard<std::mutex> lock(Internal::mutex);
        Internal::ports.insert(port);
        return true;
    }
    
    #else
    inline std::vector<Entry> list() {
        std::vector<Entry> entries;
        std::error_code ec;
        std::filesystem::directory_iterator it(get_directory(), ec);
        if (ec) return entries;
        for (const auto& item : it) {
            if (item.path().extension() != ".json") continue;
            std::ifstream file(item.path());
            rapidjson::IStreamWrapper stream(file);
            rapidjson::Document doc;
            doc.ParseStream(stream);
            if (!doc.IsObject()) continue;
            auto port = doc.FindMember("port");
            auto http = doc.FindMember("http_port");
            if (port == doc.MemberEnd() || !port->value.IsInt() || http == doc.MemberEnd() || !http->value.IsInt()) continue;
            entries.push_back({ port->value.GetInt(), http->value.GetInt() });
        }
        return entries;
    }
    #endif
}