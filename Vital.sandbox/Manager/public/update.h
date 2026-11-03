/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: update.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 02/11/2026
     Desc: Update Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Manager/public/kit.h>
#include <Vital.sandbox/Tool/http.h>
#include <Vital.sandbox/Tool/thread.h>
#include <Vital.sandbox/Tool/version.h>


////////////////////////////
// Vital: Manager: Update //
////////////////////////////

// TODO: Improve
// Shared by both sides: the main menu drives it step by step, the `update` console command runs all of it at once.
// The server additionally warns in its console whenever a newer release is out
namespace Vital::Manager {
    class Update {
        public:
            static constexpr const char* Name = "Update.manager";
            static constexpr const char* sandbox_releases = "https://github.com/ov-studio/Vital.sandbox/releases";
            static constexpr const char* kit_releases = "https://github.com/ov-studio/Vital.kit/releases";

            // Latest Vital.sandbox release, with the zip made for this side (client/server) and operating system
            struct Release {
                std::string tag, asset_name, asset_url, digest;
                uint64_t size = 0;
            };

            // A download kept in <install>/updates by an earlier session
            struct Pending {
                std::string tag, file;
            };

            // Running versions against the latest releases of Vital.sandbox and Vital.kit
            struct Status {
                Release release;
                std::string current, kit_tag, kit_local;
                bool sandbox_old = false;
                bool kit_old = false;
            };
        private:
            static constexpr const char* sandbox_src = "https://api.github.com/repos/ov-studio/Vital.sandbox/releases/latest";
            inline static std::atomic<bool> busy { false };         // One download at a time
            inline static std::atomic<bool> applying { false };     // One `update` command at a time
            #if !defined(VSDK_Client)
            static constexpr int check_interval_ms = 6 * 60 * 60 * 1000;    // GitHub API is rate limited per IP
            inline static std::mutex mutex;
            inline static std::string announced_sandbox;                    // Last versions already warned about, guarded by mutex
            inline static std::string announced_kit;
            inline static std::atomic<bool> checking { false };
            inline static Tool::Timer* timer = nullptr;
            #endif


            // Helpers //
            static void log(const std::string& mode, const std::string& message);
            static const char* get_system();
            static bool is_safe_name(const std::string& value);
            static std::string to_native(std::string path);
            static std::filesystem::path get_directory();
            static void write_pending(const std::string& tag, const std::string& file);
            static bool launch(const std::filesystem::path& src, const std::filesystem::path& zip, const std::filesystem::path& pending);
        public:
            // Helpers //
            static bool is_outdated(const std::string& local, const std::string& remote);
            static bool is_dev_build();


            // Managers //
            static Release fetch_release();
            static Status fetch_status();
            static Pending read_pending();
            static uint64_t download(const Release& release, const std::function<void(uint64_t)>& progress = nullptr);
            static bool extract(const std::string& file);
            static bool restart(const std::string& file);
            static void apply();
            #if !defined(VSDK_Client)
            static void watch();
            static void halt();
            static void check();
            #endif
    };
}
