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
#include <cstdlib>
#include <set>
#include <rapidjson/document.h>


/////////////////////////////////
// Vital: Tool: LocalServer   //
/////////////////////////////////

// TODO: Improve
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

    inline std::filesystem::path get_directory() {
        std::error_code ec;
        return std::filesystem::temp_directory_path(ec) / "vital.sandbox" / "servers";
    }

    inline std::filesystem::path get_path(int port) {
        return get_directory() / fmt::format("{}.json", port);
    }

    inline void retract(int port) {
        std::error_code ec;
        std::filesystem::remove(get_path(port), ec);
    }

    inline std::set<int>& published() {
        static std::set<int> ports;
        return ports;
    }

    inline std::mutex& published_mutex() {
        static std::mutex m;
        return m;
    }

    inline void retract_all() {
        std::lock_guard<std::mutex> lock(published_mutex());
        for (int port : published()) retract(port);
        published().clear();
    }

    inline bool publish(int port, int http_port) {
        std::error_code ec;
        std::filesystem::create_directories(get_directory(), ec);
        if (ec) return false;
        std::ofstream file(get_path(port), std::ios::trunc);
        if (!file) return false;
        file << "{\"port\":" << port << ",\"http_port\":" << http_port << "}";
        file.close();
        static bool registered = false;
        std::lock_guard<std::mutex> lock(published_mutex());
        published().insert(port);
        if (!registered) {
            registered = true;
            std::atexit([]() { retract_all(); });
        }
        return true;
    }

    inline void retract_published(int port) {
        std::lock_guard<std::mutex> lock(published_mutex());
        retract(port);
        published().erase(port);
    }

    inline std::vector<Entry> list() {
        std::vector<Entry> entries;
        std::error_code ec;
        std::filesystem::directory_iterator it(get_directory(), ec);
        if (ec) return entries;
        for (const auto& item : it) {
            if (item.path().extension() != ".json") continue;
            std::ifstream file(item.path());
            if (!file) continue;
            std::stringstream buffer;
            buffer << file.rdbuf();
            rapidjson::Document doc;
            doc.Parse(buffer.str().c_str());
            if (doc.HasParseError() || !doc.IsObject()) continue;
            if (!doc.HasMember("port") || !doc["port"].IsInt() || !doc.HasMember("http_port") || !doc["http_port"].IsInt()) continue;
            entries.push_back({ doc["port"].GetInt(), doc["http_port"].GetInt() });
        }
        return entries;
    }
}
