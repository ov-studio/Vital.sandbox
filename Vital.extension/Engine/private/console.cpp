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
                    std::cout << ANSI_BOLD << FG_GRAY << " > " << ANSI_RESET << " " << std::flush;
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

    std::string Console::ansi_rgb(const RGB& color, bool bg) {
        return ansi_rgb(color.r, color.g, color.b, bg);
    }

    std::string Console::ansi_rgb_lighten(const RGB& color, float factor) {
        return ansi_rgb(
            color.r + static_cast<int>((255 - color.r)*factor),
            color.g + static_cast<int>((255 - color.g)*factor),
            color.b + static_cast<int>((255 - color.b)*factor)
        );
    }

    Console::RGB Console::get_mode_rgb(const std::string& mode) {
        if (mode == "info")    return {80,  200, 220};
        if (mode == "success") return {100, 220, 100};
        if (mode == "warn")    return {240, 200, 80};
        if (mode == "error")   return {220, 80,  80};
        if (mode == "debug")   return {200, 100, 220};
        if (mode == "sbox")    return {80,  140, 220};
        if (mode == "system")  return {240, 160, 60};
        return {220, 220, 220};
    }

    std::string Console::format_inline(const RGB& mode_rgb, const std::string& mode_color, const std::string& content) {
        std::string result;
        size_t i = 0;
        while (i < content.size()) {
            if (content[i] == '`') {
                size_t end = content.find('`', i + 1);
                if (end != std::string::npos) {
                    result += ansi_rgb_lighten(mode_rgb, 0.35f)
                            + content.substr(i + 1, end - i - 1)
                            + mode_color;
                    i = end + 1;
                    continue;
                }
            }
            result += content[i++];
        }
        return result;
    }

    std::string Console::format_line(const RGB& mode_rgb, const std::string& mode_color, const std::string& timestamp, const std::string& mode_label, const std::string& line, bool is_continuation) {
        std::ostringstream oss;
        std::string content = line;
        const bool is_highlighted = (!content.empty() && content[0] == '>');
        if (is_highlighted) {
            content = content.substr(1);
            if (!content.empty() && content[0] == ' ') content = content.substr(1);
        }
        const std::string marker = is_highlighted
            ? (ANSI_BOLD + mode_color          + "│ " + ANSI_RESET)
            : (ANSI_DIM  + std::string(FG_GRAY) + "│ " + ANSI_RESET);
        if (!is_continuation) {
            oss << " " << ANSI_DIM << FG_GRAY << "[" << timestamp << "]" << ANSI_RESET
                << "  " << ANSI_BOLD << mode_color << "[" << mode_label << "]" << ANSI_RESET
                << "  " << (is_highlighted ? marker : "")
                << mode_color << format_inline(mode_rgb, mode_color, content) << ANSI_RESET << "\n";
        } else {
            const std::string indent(17 + mode_label.size(), ' ');
            oss << indent << marker
                << mode_color << format_inline(mode_rgb, mode_color, content) << ANSI_RESET << "\n";
        }
        return oss.str();
    }

    std::string Console::format_output(const std::string& mode, const std::string& message) {
        const Vital::Tool::Stack ts = Vital::get_timestamp();
        std::ostringstream ts_oss;
        ts_oss << std::setfill('0')
               << std::setw(2) << ts.object.at("hour").as<int32_t>()   << ":"
               << std::setw(2) << ts.object.at("minute").as<int32_t>() << ":"
               << std::setw(2) << ts.object.at("second").as<int32_t>();
        const RGB         mode_rgb   = get_mode_rgb(mode);
        const std::string mode_color = ansi_rgb(mode_rgb);
        std::string mode_label = mode;
        std::transform(mode_label.begin(), mode_label.end(), mode_label.begin(), ::toupper);
        std::ostringstream oss;
        std::istringstream stream(message);
        std::string line;
        bool first = true;
        while (std::getline(stream, line)) {
            oss << format_line(mode_rgb, mode_color, ts_oss.str(), mode_label, line, !first);
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
            document.AddMember("mode",    rapidjson::Value(mode.c_str(),    alloc), alloc);
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
        arguments.object["command"]    = tokens[0];
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