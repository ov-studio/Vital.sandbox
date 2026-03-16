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
    class Console {
        protected:
            inline static Console* singleton = nullptr;
        private:
            #if defined(Vital_SDK_Client)
            Webview* webview = nullptr;
            #else
            std::thread stdin_thread;
            std::atomic<bool> stdin_running{false};
            #if defined(Vital_SDK_MACOS) || defined(Vital_SDK_LINUX)
            struct termios stdin_termios{};
            #endif

            // ANSI //
            static constexpr const char* ANSI_RESET = "\033[0m";
            static constexpr const char* ANSI_BOLD  = "\033[1m";
            static constexpr const char* ANSI_DIM   = "\033[2m";
            static constexpr const char* FG_GRAY    = "\033[38;2;100;100;110m";

            // Color: RGB //
            struct RGB { int r, g, b; };
            static std::string      ansi_rgb(int r, int g, int b, bool bg = false);
            static std::string      ansi_rgb(const RGB& color, bool bg = false);
            static std::string      ansi_rgb_lighten(const RGB& color, float factor = 0.3f);
            static RGB              get_mode_rgb(const std::string& mode);
            static std::string      format_inline(const RGB& mode_rgb, const std::string& mode_color, const std::string& content);
            static std::string      format_line(const RGB& mode_rgb, const std::string& mode_color, const std::string& timestamp, const std::string& mode_label, const std::string& line, bool is_continuation);
            static std::string      format_output(const std::string& mode, const std::string& message);
            #endif
        public:
            // Instantiators //
            Console();
            ~Console();

            // Utils //
            static Console* get_singleton();
            static void free_singleton();

            // APIs //
            void print(const std::string& mode, const std::string& message);
            void execute(const std::string& input);

            // Events //
            #if defined(Vital_SDK_Client)
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