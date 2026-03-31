/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: console.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Console Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/index.h>
#if defined(Vital_SDK_Client)
#include <Vital.extension/Engine/public/webview.h>
#endif


/////////////////////////////
// Vital: Engine: Console //
/////////////////////////////

namespace Vital::Engine {
    class Console : public godot::Control {
        protected:
            inline static Console* singleton = nullptr;
        private:
            #if defined(Vital_SDK_Client)
            Webview* webview = nullptr;
            #else
            std::mutex stdout_mutex;
            std::thread stdin_thread;
            std::atomic<bool> stdin_running{false};
            std::string stdin_buffer;
            std::vector<std::string> stdin_history;
            int stdin_history_index = -1;
            #if defined(Vital_SDK_WINDOWS)
            DWORD stdin_original_mode = 0;
            #elif defined(Vital_SDK_MACOS) || defined(Vital_SDK_LINUX)
            struct termios stdin_termios{};
            #endif

            static constexpr const char* ANSI_RESET = "\033[0m";
            static constexpr const char* ANSI_BOLD = "\033[1m";
            static constexpr const char* ANSI_DIM = "\033[2m";
            static constexpr const char* FG_GRAY = "\033[38;2;100;100;110m";
            static std::string ansi_rgb(int r, int g, int b);
            static std::string ansi_rgb(const Vital::Tool::Stack& color);
            static std::string ansi_rgb_lighten(const Vital::Tool::Stack& color, float factor = 0.3f);
            static std::string format_inline(const Vital::Tool::Stack& mode_rgb, const std::string& content);
            static std::string format_line(const Vital::Tool::Stack& mode_rgb, const std::string& timestamp, const std::string& mode_label, const std::string& line, bool is_continuation);
            static std::string format_output(const std::string& mode, const std::string& message);
            void format_input_prompt();
            #endif
            static std::string fetch_mode_label(const std::string& mode);
            static std::string fetch_mode_badge(const std::string& mode);
            static Vital::Tool::Stack fetch_mode_color(const std::string& mode);
            static std::string fetch_version();
            static std::string fetch_help();
        public:
            // Instantiators //
            Console();
            ~Console();


            // Utils //
            static Console* get_singleton();
            static void free_singleton();


            // APIs //
            void init();
            void execute(const std::string& input);
            void print(const std::string& mode, const std::string& message);
            void clear(bool signal = false);
            #if !defined(Vital_SDK_Client)
            void shutdown();
            #else
            void toggle();
            #endif


            // Events //
            #if defined(Vital_SDK_Client)
            bool on_key(int keycode);
            void on_message(godot::String message);
            #endif
    };
}

namespace Vital {
    template<typename... Args>
    inline void print(const std::string& mode, Args&&... args) {
        std::ostringstream oss;
        bool first = true;
        ((oss << (first ? (first = false, "") : " ") << std::forward<Args>(args)), ...);
        Engine::Console::get_singleton() -> print(mode, oss.str());
    }
}