#pragma once
#include <Vital.sandbox/vital.hpp>
#include <RmlUi/Core/SystemInterface.h>
#include <chrono>

class RmlGodotSystem final : public Rml::SystemInterface {
public:
    double GetElapsedTime() override {
        static auto start = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(now - start).count();
    }

    bool LogMessage(Rml::Log::Type, const Rml::String& message) override {
        printf("[RmlUi] %s\n", message.c_str());
        return true;
    }
};
