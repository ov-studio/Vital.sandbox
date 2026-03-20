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
#include <Vital.extension/Sandbox/index.h>


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
                HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
                HANDLE hStdin  = GetStdHandle(STD_INPUT_HANDLE);
                CONSOLE_FONT_INFOEX fontInfo = {};
                fontInfo.cbSize = sizeof(fontInfo);
                fontInfo.dwFontSize.Y = 20;
                fontInfo.FontFamily = FF_DONTCARE;
                fontInfo.FontWeight = FW_NORMAL;
                wcscpy_s(fontInfo.FaceName, L"Cascadia Code");
                if (!SetCurrentConsoleFontEx(hStdout, FALSE, &fontInfo)) {
                    wcscpy_s(fontInfo.FaceName, L"Consolas");
                    SetCurrentConsoleFontEx(hStdout, FALSE, &fontInfo);
                }
                COORD size = {120, 9999};
                SetConsoleScreenBufferSize(hStdout, size);
                SMALL_RECT rect = {0, 0, 119, 29};
                SetConsoleWindowInfo(hStdout, TRUE, &rect);
                DWORD out_mode, in_mode;
                GetConsoleMode(hStdout, &out_mode);
                SetConsoleMode(hStdout, out_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);
                GetConsoleMode(hStdin, &in_mode);
                SetConsoleMode(hStdin, in_mode | ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
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
                    {
                        std::lock_guard<std::mutex> lock(stdout_mutex);
                        std::cout << ANSI_BOLD << FG_GRAY << " > " << ANSI_RESET << " " << std::flush;
                    }
                    if (!std::getline(std::cin, line)) break;
                    {
                        std::lock_guard<std::mutex> lock(stdout_mutex);
                        std::cout << "\033[1A\033[2K" << std::flush;
                    }
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

    std::string Console::fetch_mode_label(const std::string& mode) {
        const auto label = Vital::Tool::fetch_config("log", mode, "label");
        if (label.is<std::string>()) return label.as<std::string>();
        return mode;
    }

    std::string Console::fetch_mode_badge(const std::string& mode) {
        const auto badge = Vital::Tool::fetch_config("log", mode, "badge");
        if (badge.is<std::string>()) return badge.as<std::string>();
        std::string upper = mode;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        return upper;
    }

    Vital::Tool::Stack Console::fetch_mode_color(const std::string& mode) {
        Vital::Tool::Stack color;
        const auto r = Vital::Tool::fetch_config("log", mode, "color", 0);
        const auto g = Vital::Tool::fetch_config("log", mode, "color", 1);
        const auto b = Vital::Tool::fetch_config("log", mode, "color", 2);
        if (r.is<int32_t>() && g.is<int32_t>() && b.is<int32_t>()) color.array = {r.as<int32_t>(), g.as<int32_t>(), b.as<int32_t>()};
        else color.array = {220, 220, 220};
        return color;
    }

    #if !defined(Vital_SDK_Client)
    std::string Console::ansi_rgb(int r, int g, int b) {
        std::ostringstream oss;
        oss << "\033[38;2;" << r << ";" << g << ";" << b << "m";
        return oss.str();
    }

    std::string Console::ansi_rgb(const Vital::Tool::Stack& color) {
        return ansi_rgb(
            color.array[0].as<int32_t>(),
            color.array[1].as<int32_t>(),
            color.array[2].as<int32_t>()
        );
    }

    std::string Console::ansi_rgb_lighten(const Vital::Tool::Stack& color, float factor) {
        const int r = color.array[0].as<int32_t>();
        const int g = color.array[1].as<int32_t>();
        const int b = color.array[2].as<int32_t>();
        return ansi_rgb(
            r + static_cast<int>((255 - r)*factor),
            g + static_cast<int>((255 - g)*factor),
            b + static_cast<int>((255 - b)*factor)
        );
    }

    std::string Console::format_inline(const Vital::Tool::Stack& mode_rgb, const std::string& content) {
        const std::string mode_color = ansi_rgb(mode_rgb);
        std::string result;
        size_t i = 0;
        while (i < content.size()) {
            if (content[i] == '`') {
                size_t end = content.find('`', i + 1);
                if (end != std::string::npos) {
                    result += ansi_rgb_lighten(mode_rgb, 0.35f) + content.substr(i + 1, end - i - 1) + mode_color;
                    i = end + 1;
                    continue;
                }
            }
            result += content[i++];
        }
        return result;
    }

    std::string Console::format_line(const Vital::Tool::Stack& mode_rgb, const std::string& timestamp, const std::string& mode_label, const std::string& line, bool is_continuation) {
        const std::string mode_color = ansi_rgb(mode_rgb);
        std::ostringstream oss;
        std::string content = line;
        const bool is_highlighted = (!content.empty() && content[0] == '>');
        if (is_highlighted) {
            content = content.substr(1);
            if (!content.empty() && content[0] == ' ') content = content.substr(1);
        }
        const std::string marker = is_highlighted ? (ANSI_BOLD + mode_color + "│ " + ANSI_RESET) : (ANSI_DIM + std::string(FG_GRAY) + "│ " + ANSI_RESET);
        if (!is_continuation) {
            oss << " " << ANSI_DIM << FG_GRAY << "[" << timestamp << "]" << ANSI_RESET
                << "  " << ANSI_BOLD << mode_color << "[" << mode_label << "]" << ANSI_RESET
                << "  " << (is_highlighted ? marker : "")
                << mode_color << format_inline(mode_rgb, content) << ANSI_RESET << "\n";
        }
        else {
            if (content.empty()) return "";
            const std::string indent(17 + mode_label.size(), ' ');
            oss << indent << marker << mode_color << format_inline(mode_rgb, content) << ANSI_RESET << "\n";
        }
        return oss.str();
    }

    std::string Console::format_output(const std::string& mode, const std::string& message) {
        const Vital::Tool::Stack ts = Vital::get_timestamp();
        const Vital::Tool::Stack mode_rgb = fetch_mode_color(mode);
        auto mode_badge = fetch_mode_badge(mode);
        std::ostringstream ts_oss;
        ts_oss << std::setfill('0')
               << std::setw(2) << ts.object.at("hour").as<int32_t>() << ":"
               << std::setw(2) << ts.object.at("minute").as<int32_t>() << ":"
               << std::setw(2) << ts.object.at("second").as<int32_t>();
        std::ostringstream oss;
        std::istringstream stream(message);
        std::string line;
        bool first = true;
        while (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;
            oss << format_line(mode_rgb, ts_oss.str(), mode_badge, line, !first);
            first = false;
        }
        oss << "\n";
        return oss.str();
    }
    #endif


    // APIs //
    void Console::init() {
        #if defined(Vital_SDK_Client)
        rapidjson::Document document;
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.SetObject();
        auto& alloc = document.GetAllocator();
        auto make_color = [&](const Vital::Tool::Stack& color) {
            rapidjson::Value result(rapidjson::kArrayType);
            result.PushBack(color.array[0].as<int32_t>(), alloc);
            result.PushBack(color.array[1].as<int32_t>(), alloc);
            result.PushBack(color.array[2].as<int32_t>(), alloc);
            return result;
        };
        document.AddMember("action", "init", alloc);
        rapidjson::Value types(rapidjson::kObjectType);
        const auto levels = Vital::Tool::fetch_config_base("log");
        if (levels) {
            for (auto it = levels -> MemberBegin(); it != levels -> MemberEnd(); ++it) {
                const std::string mode = it -> name.GetString();
                const auto label = fetch_mode_label(mode);
                const auto badge = fetch_mode_badge(mode);
                const auto color = fetch_mode_color(mode);
                const auto priority = Vital::Tool::fetch_config("log", mode, "priority");
                rapidjson::Value entry(rapidjson::kObjectType);
                entry.AddMember("label", rapidjson::Value(label.c_str(), alloc), alloc);
                entry.AddMember("badge", rapidjson::Value(badge.c_str(), alloc), alloc);
                entry.AddMember("priority", priority.is<int32_t>() ? priority.as<int32_t>() : 99, alloc);
                entry.AddMember("color", make_color(color), alloc);
                rapidjson::Value key(mode.c_str(), alloc);
                types.AddMember(key, entry, alloc);
            }
            document.AddMember("types", types, alloc);
            document.Accept(writer);
            webview -> emit(buffer.GetString());
        }
        #endif
    }

    void Console::execute(const std::string& input) {
        std::istringstream iss(input);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) tokens.push_back(token);
        if (tokens.empty()) return;
        Sandbox::get_singleton() -> signal("vital.sandbox:console_input",
            Vital::Tool::StackValue(tokens[0]),
            Vital::Tool::StackValue(std::vector<std::string>(tokens.begin() + 1, tokens.end()))
        );
    }

    void Console::print(const std::string& mode, const std::string& message) {
        if (!Vital::Log::is_type(mode)) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
        if (message.empty()) return;
        #if defined(Vital_SDK_Client)
            rapidjson::Document document;
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.SetObject();
            auto& alloc = document.GetAllocator();
            document.AddMember("action", "print", alloc);
            document.AddMember("mode", rapidjson::Value(mode.c_str(), alloc), alloc);
            document.AddMember("message", rapidjson::Value(message.c_str(), alloc), alloc);
            document.Accept(writer);
            webview -> emit(buffer.GetString());
        #else
            {
                std::lock_guard<std::mutex> lock(stdout_mutex);
                std::cout << "\033[2K\r" << format_output(mode, message) << ANSI_BOLD << FG_GRAY << " > " << ANSI_RESET << " " << std::flush;
            }
        #endif
    }


    // Events //
    #if defined(Vital_SDK_Client)
    void Console::on_message(godot::String message) {
        rapidjson::Document document;
        document.Parse(to_std_string(message).c_str());
        if (document.HasParseError() || !document.HasMember("action")) return;
        std::string action = document["action"].GetString();
        if (action == "ready") init();
        else if (action == "input") {
            if (!document.HasMember("message") || !document["message"].IsString()) return;
            execute(document["message"].GetString());
        }
    }
    #endif
}