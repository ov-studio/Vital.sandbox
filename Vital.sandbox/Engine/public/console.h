/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: console.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Console Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/webview.h>
#endif


/////////////////////////////
// Vital: Engine: Console //
/////////////////////////////

namespace Vital::Engine {
    class Console : public godot::Control, public Tool::Base<Console> {
        friend class Tool::Base<Console>;
        private:
            #if defined(VSDK_Client)
            Webview* webview = nullptr;
            std::mutex pre_ready_mutex;
            std::atomic<bool> webview_ready { false };
            std::vector<std::pair<std::string, std::string>> pre_ready_queue;
            #else
            std::mutex stdout_mutex;
            std::thread stdin_thread;
            std::atomic<bool> stdin_running { false };
            std::string stdin_buffer;
            std::vector<std::string> stdin_history;
            int stdin_history_idx = -1;
            #if defined(VSDK_WINDOWS)
            DWORD stdin_original_mode = 0;
            #elif defined(VSDK_MACOS) || defined(VSDK_LINUX)
            struct termios stdin_termios{};
            #endif
            #endif
            std::thread log_thread;
            std::atomic<bool> log_running { false };

            struct Internal {
                struct NativeLogEntry {
                    const char* prefix;
                    const char* mode;
                    bool strip;
                    const char* label;
                };

                static inline constexpr NativeLogEntry native_logs[] = {
                    { "[Vital.wry]",          "warn",  true,  "Vital.wry"          },
                    { "[Vital.wry.protocol]", "warn",  true,  "Vital.wry.protocol" },
                    { "[Godot ERR]",          "error", true,  "Godot.engine"       },
                    { "SHADER ERROR:",        "error", true,  "Godot.engine"       },
                    { "ERROR:",               "error", true,  "Godot.engine"       },
                    { "WARNING:",             "warn",  true,  "Godot.engine"       },
                    { "  at:",                "error", false, "Godot.engine"       },
                    { "\tat:",                "error", false, "Godot.engine"       }
                };

                
                // Helpers //
                static std::string derive_label(const NativeLogEntry& e);
                static std::string fetch_mode_label(const std::string& mode);
                static std::string fetch_mode_badge(const std::string& mode);
                static Tool::Stack fetch_mode_color(const std::string& mode);
                static std::string fetch_version();
                static std::string fetch_help();
                static void parse_log_line(const std::string& line);
                #if !defined(VSDK_Client)
                static constexpr const char* ANSI_RESET = "\033[0m";
                static constexpr const char* ANSI_BOLD = "\033[1m";
                static constexpr const char* ANSI_DIM = "\033[2m";
                static constexpr const char* FG_GRAY = "\033[38;2;100;100;110m";
                static std::string fetch_info();
                static std::string ansi_rgb(int r, int g, int b);
                static std::string ansi_rgb(const Tool::Stack& color);
                static std::string ansi_rgb_lighten(const Tool::Stack& color, float factor = 0.3f);
                static std::string format_inline(const Tool::Stack& mode_rgb, const std::string& content);
                static std::string format_line(const Tool::Stack& mode_rgb, const std::string& timestamp, const std::string& mode_label, const std::string& line, bool is_continuation);
                static std::string format_output(const std::string& mode, const std::string& message);
                #endif
            };


            // Instantiators //
            Console();
            ~Console();
        public:
            static constexpr const char* Name = "Console.engine";


            // Managers //
            #if defined(VSDK_Client)
            bool is_ready();
            bool is_visible();
            #endif
            void init();
            void ready();
            void update();
            void execute(const std::string& input);
            void print(const std::string& mode, const std::string& message);
            void clear(bool signal = false);
            void teardown();
            #if defined(VSDK_Client)
            void toggle();
            #endif


            // Events //
            #if defined(VSDK_Client)
            void on_message(godot::String message);
            #endif
    };
}