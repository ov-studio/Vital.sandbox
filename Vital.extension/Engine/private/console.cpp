/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: console.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Console Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/console.h>


/////////////////////////////
// Vital: Engine: Console //
/////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Console::Console() {
        #if defined(Vital_SDK_Client)
            webview = Webview::create();
            webview -> set_position({0, 0});
            webview -> set_visible(true);
            webview -> set_fullscreen(true);
            webview -> set_transparent(true);
            webview -> set_autoplay(false);
            webview -> set_zoomable(false);
            webview -> set_devtools_visible(false);
            webview -> load_html(Vital::Tool::fetch_module("console"));
            webview -> set_message_handler([this](godot::String message) {
                this -> on_message(message);
            });
        #else
            #if defined(Vital_SDK_WINDOWS)
                AllocConsole();
                SetConsoleTitleA("Vital.server");
                SetConsoleOutputCP(CP_UTF8);
                SetConsoleCP(CP_UTF8);
                freopen("CONOUT$", "w", stdout);
                freopen("CONOUT$", "w", stderr);
                freopen("CONIN$",  "r", stdin);
                HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
                DWORD mode;
                GetConsoleMode(hStdin, &mode);
                SetConsoleMode(hStdin, mode | ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
                HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
                DWORD out_mode;
                GetConsoleMode(hStdout, &out_mode);
                SetConsoleMode(hStdout, out_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            #elif defined(Vital_SDK_MACOS) || defined(Vital_SDK_LINUX)
                tcgetattr(STDIN_FILENO, &stdin_termios);
                struct termios term = stdin_termios;
                term.c_lflag |= (ICANON | ECHO | ECHOE | ECHOK | ISIG);
                tcsetattr(STDIN_FILENO, TCSANOW, &term);
            #endif
            stdin_running = true;
            stdin_thread = std::thread([this]() {
                std::string line;
                while (stdin_running) {
                    std::cout
                        << ANSI_BOLD << FG_GRAY << " > " << ANSI_RESET
                        << " " << std::flush;
                    if (!std::getline(std::cin, line)) break;
                    std::cout << "\033[1A\033[2K" << std::flush;
                    execute(line);
                }
            });
            stdin_thread.detach();
        #endif
    }

    Console::~Console() {
        #if defined(Vital_SDK_Client)
            if (!webview) return;
            webview -> destroy();
            webview = nullptr;
        #else
            stdin_running = false;
            #if defined(Vital_SDK_WINDOWS)
                FreeConsole();
            #elif defined(Vital_SDK_MACOS) || defined(Vital_SDK_LINUX)
                tcsetattr(STDIN_FILENO, TCSANOW, &stdin_termios);
            #endif
        #endif
    }


    // Utils //
    Console* Console::get_singleton() {
        if (!singleton) singleton = memnew(Console);
        return singleton;
    }

    void Console::free_singleton() {
        if (!singleton) return;
        memdelete(singleton);
        singleton = nullptr;
    }

    #if !defined(Vital_SDK_Client)
    std::string Console::ansi_rgb(int r, int g, int b, bool bg) {
        std::ostringstream oss;
        oss << "\033[" << (bg ? 48 : 38) << ";2;" << r << ";" << g << ";" << b << "m";
        return oss.str();
    }

    std::string Console::get_timestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
        #if defined(Vital_SDK_WINDOWS)
            localtime_s(&tm, &t);
        #else
            localtime_r(&t, &tm);
        #endif
        std::ostringstream oss;
        oss << std::setfill('0')
            << std::setw(2) << tm.tm_hour << ":"
            << std::setw(2) << tm.tm_min  << ":"
            << std::setw(2) << tm.tm_sec;
        return oss.str();
    }

    std::string Console::get_mode_color(const std::string& mode) {
        if (mode == "info")    return FG_CYAN;
        if (mode == "success") return FG_GREEN;
        if (mode == "warn")    return FG_YELLOW;
        if (mode == "error")   return FG_RED;
        if (mode == "debug")   return FG_MAGENTA;
        if (mode == "sbox")    return FG_BLUE;
        if (mode == "system")  return FG_ORANGE;
        return FG_WHITE;
    }

    std::string Console::format_line(const std::string& mode_color, const std::string& timestamp, const std::string& mode_label, const std::string& line, bool is_continuation) {
        std::ostringstream oss;

        std::string content = line;
        bool is_highlighted = (!content.empty() && content[0] == '>');
        if (is_highlighted) {
            content = content.substr(1);
            if (!content.empty() && content[0] == ' ') content = content.substr(1);
        }

        if (!is_continuation) {
            oss << " "
                << ANSI_DIM  << FG_GRAY    << "[" << timestamp << "]" << ANSI_RESET
                << "  "
                << ANSI_BOLD << mode_color << "[" << mode_label << "]" << ANSI_RESET
                << "  "
                << mode_color              << content    << ANSI_RESET
                << "\n";
        }
        else {
                const size_t indent_size = 17 + mode_label.size();
                const std::string indent(indent_size, ' ');
                const std::string marker = is_highlighted
                    ? (std::string(ANSI_BOLD) + mode_color + "│ " + ANSI_RESET)
                    : (std::string(ANSI_DIM)  + FG_GRAY    + "│ " + ANSI_RESET);
                oss << indent
                    << marker
                    << mode_color << content << ANSI_RESET
                    << "\n";
            }
        return oss.str();
    }

    std::string Console::format_output(const std::string& mode, const std::string& message) {
        const std::string timestamp  = get_timestamp();
        const std::string mode_color = get_mode_color(mode);
        std::string mode_label = mode;
        std::transform(mode_label.begin(), mode_label.end(), mode_label.begin(), ::toupper);

        std::ostringstream oss;
        std::istringstream stream(message);
        std::string line;
        bool first = true;
        while (std::getline(stream, line)) {
            oss << format_line(mode_color, timestamp, mode_label, line, !first);
            first = false;
        }
        oss << "\n";
        return oss.str();
    }
    #endif


    // APIs //
    void Console::print(const std::string& mode, const std::string& message) {
        if (mode.empty() || message.empty()) return;
        #if defined(Vital_SDK_Client)
            rapidjson::Document document;
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.SetObject();
            auto& alloc = document.GetAllocator();
            document.AddMember("action",  "print", alloc);
            document.AddMember("mode", rapidjson::Value(mode.c_str(), alloc), alloc);
            document.AddMember("message", rapidjson::Value(message.c_str(), alloc), alloc);
            document.Accept(writer);
            webview -> emit(buffer.GetString());
        #else
            std::cout << format_output(mode, message) << std::flush;
        #endif
    }

    void Console::execute(const std::string& input) {
        std::istringstream iss(input);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) tokens.push_back(token);
        if (tokens.empty()) return;
        Vital::Tool::Stack arguments;
        arguments.object["command"] = tokens[0];
        arguments.object["parameters"] = std::vector<std::string>(tokens.begin() + 1, tokens.end());
        Vital::Tool::Event::emit("vital.sandbox:console_input", arguments);
    }


    // Events //
    #if defined(Vital_SDK_Client)
    void Console::on_message(godot::String message) {
        rapidjson::Document document;
        document.Parse(to_std_string(message).c_str());
        if (document.HasParseError() || !document.HasMember("action")) return;
        std::string action = document["action"].GetString();
        if (action == "input") {
            if (!document.HasMember("message") || !document["message"].IsString()) return;
            execute(document["message"].GetString());
        }
    }
    #endif
}