/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: updater.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 02/11/2026
     Desc: Server Updater Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if !defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Manager/public/kit.h>
#include <Vital.sandbox/Tool/http.h>
#include <Vital.sandbox/Tool/thread.h>
#include <Vital.sandbox/Tool/version.h>


///////////////////////////////
// Vital: Manager: Updater //
///////////////////////////////

// TODO: Improve
// Server counterpart of the main menu updater: warns in the console when a newer release is out, and `update` installs it
namespace Vital::Manager {
    class Updater {
        public:
            static constexpr const char* Name = "Updater.manager";
        private:
            static constexpr const char* sandbox_src = "https://api.github.com/repos/ov-studio/Vital.sandbox/releases/latest";
            static constexpr const char* sandbox_releases = "https://github.com/ov-studio/Vital.sandbox/releases";
            static constexpr int check_interval_ms = 6 * 60 * 60 * 1000;    // GitHub API is rate limited per IP
            inline static std::mutex mutex;
            inline static std::string announced_sandbox;                    // Last versions already warned about, guarded by mutex
            inline static std::string announced_kit;
            inline static std::atomic<bool> checking { false };
            inline static std::atomic<bool> applying { false };
            inline static Tool::Timer* timer = nullptr;

            struct Release {
                std::string tag, asset_name, asset_url, digest;
                uint64_t size = 0;
            };


            // Helpers //
            static void log(const std::string& mode, const std::string& message);
            static const char* get_system();
            static bool is_outdated(const std::string& local, const std::string& remote);
            static bool is_dev_build();
            static bool is_safe_name(const std::string& value);
            static std::filesystem::path get_directory();
            static std::string to_native(std::string path);
            static Release fetch_release();
            static void write_pending(const std::string& tag, const std::string& file);
            static std::string read_pending(const std::string& tag);
            static uint64_t download(const Release& release);
            static bool launch(const std::filesystem::path& src, const std::filesystem::path& zip, const std::filesystem::path& pending);
        public:
            // Managers //
            static void watch();
            static void halt();
            static void check();
            static void apply();
    };
}
#endif
