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
#include <Vital.extension/Engine/public/core.h>
#include <Vital.extension/Engine/public/console.h>
#include <Vital.extension/Engine/public/resource.h>
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
                fontInfo.dwFontSize.Y = 19;
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
                stdin_original_mode = in_mode;
                SetConsoleMode(hStdin, ENABLE_PROCESSED_INPUT);
            #elif defined(Vital_SDK_MACOS) || defined(Vital_SDK_LINUX)
                tcgetattr(STDIN_FILENO, &stdin_termios);
                struct termios term = stdin_termios;
                term.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK);
                term.c_lflag |= ISIG;
                term.c_cc[VMIN]  = 1;
                term.c_cc[VTIME] = 0;
                tcsetattr(STDIN_FILENO, TCSANOW, &term);
            #endif

            stdin_running = true;
            stdin_thread = std::thread([this]() {
                format_input_prompt();
                while (stdin_running) {
                    #if defined(Vital_SDK_WINDOWS)
                        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
                        INPUT_RECORD rec;
                        DWORD count;
                        if (!ReadConsoleInputA(hStdin, &rec, 1, &count)) break;
                        if (rec.EventType != KEY_EVENT || !rec.Event.KeyEvent.bKeyDown) continue;
                        char ch = rec.Event.KeyEvent.uChar.AsciiChar;
                        WORD vk  = rec.Event.KeyEvent.wVirtualKeyCode;
                        if (vk == VK_RETURN) {
                            std::string line;
                            {
                                std::lock_guard<std::mutex> lock(stdout_mutex);
                                line = stdin_buffer;
                                stdin_buffer.clear();
                                std::cout << "\r\033[2K\033[1B\033[2K\033[1A" << std::flush;
                            }
                            if (!line.empty()) execute(line);
                            format_input_prompt();
                        } else if (vk == VK_BACK) {
                            {
                                std::lock_guard<std::mutex> lock(stdout_mutex);
                                if (!stdin_buffer.empty()) stdin_buffer.pop_back();
                            }
                            format_input_prompt();
                        } else if (ch >= 0x20 && ch < 0x7F) {
                            {
                                std::lock_guard<std::mutex> lock(stdout_mutex);
                                stdin_buffer += ch;
                            }
                            format_input_prompt();
                        }
                    #elif defined(Vital_SDK_MACOS) || defined(Vital_SDK_LINUX)
                        char ch;
                        ssize_t n = ::read(STDIN_FILENO, &ch, 1);
                        if (n <= 0) break;
                        if (ch == '\n' || ch == '\r') {
                            std::string line;
                            {
                                std::lock_guard<std::mutex> lock(stdout_mutex);
                                line = stdin_buffer;
                                stdin_buffer.clear();
                                std::cout << "\r\033[2K\033[1B\033[2K\033[1A" << std::flush;
                            }
                            if (!line.empty()) execute(line);
                            format_input_prompt();
                        }
                        else if (ch == 0x7F || ch == '\b') {
                            {
                                std::lock_guard<std::mutex> lock(stdout_mutex);
                                if (!stdin_buffer.empty()) stdin_buffer.pop_back();
                            }
                            format_input_prompt();
                        }
                        else if (ch >= 0x20 && ch < 0x7F) {
                            {
                                std::lock_guard<std::mutex> lock(stdout_mutex);
                                stdin_buffer += ch;
                            }
                            format_input_prompt();
                        }
                    #endif
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
                HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
                SetConsoleMode(hStdin, stdin_original_mode);
                FreeConsole();
            #elif defined(Vital_SDK_MACOS) || defined(Vital_SDK_LINUX)
                tcsetattr(STDIN_FILENO, TCSANOW, &stdin_termios);
            #endif
        #endif
    }


    // Helpers //
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
                << "   " << ANSI_BOLD << mode_color << "[" << mode_label << "]" << ANSI_RESET
                << "  " << (is_highlighted ? marker : "")
                << mode_color << format_inline(mode_rgb, content) << ANSI_RESET << "\n";
        }
        else {
            if (content.empty()) return "";
            const std::string indent(18 + mode_label.size(), ' ');
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

    void Console::format_input_prompt() {
        std::lock_guard<std::mutex> lock(stdout_mutex);
        const int cursor_col = 5 + static_cast<int>(stdin_buffer.size());
        std::ostringstream prompt_oss;
        prompt_oss << "\r\033[J"
                   << ANSI_BOLD << FG_GRAY << " > " << ANSI_RESET << " "
                   << stdin_buffer
                   << "\n"
                   << "\033[1A"
                   << "\033[" << cursor_col << "G";
        std::cout << prompt_oss.str() << std::flush;
    }
    #endif

    std::string Console::fetch_mode_label(const std::string& mode) {
        const auto label = Vital::Tool::fetch_json_value("config/log", mode, "label");
        if (label.is<std::string>()) return label.as<std::string>();
        return mode;
    }

    std::string Console::fetch_mode_badge(const std::string& mode) {
        const auto badge = Vital::Tool::fetch_json_value("config/log", mode, "badge");
        if (badge.is<std::string>()) return badge.as<std::string>();
        std::string upper = mode;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        return upper;
    }

    Vital::Tool::Stack Console::fetch_mode_color(const std::string& mode) {
        const auto r = Vital::Tool::fetch_json_value("config/log", mode, "color", 0);
        const auto g = Vital::Tool::fetch_json_value("config/log", mode, "color", 1);
        const auto b = Vital::Tool::fetch_json_value("config/log", mode, "color", 2);
        return Vital::Tool::Stack(
            r.is<int32_t>() && g.is<int32_t>() && b.is<int32_t>()
            ? std::initializer_list<Vital::Tool::StackValue>{ r.as<int32_t>(), g.as<int32_t>(), b.as<int32_t>() }
            : std::initializer_list<Vital::Tool::StackValue>{ int32_t(220), int32_t(220), int32_t(220) }
        );
    }

    std::string Console::fetch_help() {
        std::ostringstream oss;
        oss << "Available Commands:\n";
        auto append_section = [&](const std::string& section, const std::string& label) {
            auto& help = Vital::Tool::fetch_json("config/help");
            if (help.HasParseError() || !help.HasMember(section.c_str())) return;
            const auto& cmds = help[section.c_str()];
            if (!cmds.IsObject()) return;
            oss << "\n" << indent(1) << label << ":\n";
            for (auto it = cmds.MemberBegin(); it != cmds.MemberEnd(); ++it) {
                std::string cmd = it -> name.GetString();
                std::string syntax = it -> value.HasMember("syntax") && it -> value["syntax"].IsString() ? it -> value["syntax"].GetString() : "";
                std::string desc = it -> value.HasMember("desc")   && it -> value["desc"].IsString()   ? it -> value["desc"].GetString()   : "";
                std::string full_cmd = syntax.empty() ? fmt::format("`{}`", cmd) : fmt::format("`{}` {}", cmd, syntax);
                oss << fmt::format("{}{} — {}\n", indent(2), full_cmd, desc);
            }
        };
        append_section("shared", "General");
        #if !defined(Vital_SDK_Client)
        append_section("server", "Server");
        #endif
        #if defined(Vital_SDK_Client)
        append_section("client", "Client");
        #endif
        return oss.str();
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
        auto& levels = Vital::Tool::fetch_json("config/log");
        if (!levels.HasParseError() && levels.IsObject()) {
            for (auto it = levels.MemberBegin(); it != levels.MemberEnd(); ++it) {
                const std::string mode = it -> name.GetString();
                const auto label = fetch_mode_label(mode);
                const auto badge = fetch_mode_badge(mode);
                const auto color = fetch_mode_color(mode);
                const auto priority = Vital::Tool::fetch_json_value("config/log", mode, "priority");
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
        auto exec = [&](const std::vector<std::string>& tokens) -> bool {
            if (tokens[0] == "help") { print("sbox", fetch_help()); return true; }
            if (tokens[0] == "version") { print("sbox", "Version: " + Vital::Build.to_string()); return true; }
            if (tokens[0] == "clear") { clear(); return true; }
            #if !defined(Vital_SDK_Client)
            if (tokens[0] == "refresh") { Vital::Engine::ResourceManager::get_singleton() -> scan(); return true; }
            if (tokens[0] == "start") { Vital::Engine::ResourceManager::get_singleton() -> start(tokens[1]); return true; }
            if (tokens[0] == "stop") { Vital::Engine::ResourceManager::get_singleton() -> stop(tokens[1]); return true; }
            if (tokens[0] == "restart") { Vital::Engine::ResourceManager::get_singleton() -> restart(tokens[1]); return true; }
            if (tokens[0] == "shutdown") { shutdown(); return true; }
            #endif
            return false;
        };
        if (exec(tokens)) return;
        Vital::Tool::Stack arguments;
        arguments.array.reserve(tokens.size() - 1);
        for (std::size_t i = 1; i < tokens.size(); ++i) {
            arguments.array.emplace_back(tokens[i]);
        }
        Sandbox::get_singleton() -> signal("vital.sandbox:console_input",
            Vital::Tool::StackValue(tokens[0]),
            Vital::Tool::StackValue(std::move(arguments))
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
                const int cursor_col = 5 + static_cast<int>(stdin_buffer.size());
                std::ostringstream out;
                out << "\r\033[J";
                out << format_output(mode, message);
                out << ANSI_BOLD << FG_GRAY << " > " << ANSI_RESET << " "
                    << stdin_buffer
                    << "\n"
                    << "\033[1A"
                    << "\033[" << cursor_col << "G";
                std::cout << out.str() << std::flush;
            }
        #endif
    }

    void Console::clear(bool signal) {
        if (signal) return print("sbox", "Console cleared successfully!");
        #if defined(Vital_SDK_Client)
            rapidjson::Document document;
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.SetObject();
            auto& alloc = document.GetAllocator();
            document.AddMember("action", "clear", alloc);
            document.Accept(writer);
            webview -> emit(buffer.GetString());
        #else
            {
                std::lock_guard<std::mutex> lock(stdout_mutex);
                std::cout << "\033[3J\033[2J\033[H" << std::flush;
                std::cout << "\033[3J\033[2J\033[H" << std::flush;
            }
            clear(true);
        #endif
    }

    #if !defined(Vital_SDK_Client)
    void Console::shutdown() {
        print("sbox", "Server shutting down...");
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        stdin_running = false;
        Core::get_singleton() -> call_deferred("free_singleton");
    }
    #endif


    // Events //
    #if defined(Vital_SDK_Client)
    void Console::on_message(godot::String message) {
        rapidjson::Document document;
        document.Parse(to_std_string(message).c_str());
        if (document.HasParseError() || !document.HasMember("action")) return;
        std::string action = document["action"].GetString();
        if (action == "ready") init();
        else if (action == "clear") clear(true);
        else if (action == "input") {
            if (!document.HasMember("message") || !document["message"].IsString()) return;
            execute(document["message"].GetString());
        }
    }
    #endif
}