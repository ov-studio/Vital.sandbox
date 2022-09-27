﻿#pragma once
#include <Sandbox/lua/public/index.h>
#include <System/public/filesystem.h>

const std::string vSignature = "VitalSDK";
void outputConsole(std::string message) { std::cout << "\nVital.bundler | " << message; }
void genPackage(std::string name, std::string entry, std::vector<std::string> modules) {
    std::string rwBuffer = "std::vector<std::string> buffer = {";
    outputConsole("Packaging " + name + "...");
    for (auto i : modules) {
        std::string path = entry + i;
        if (!Vital::FileSystem::exists(path)) {
            std::string error = "Invalid File: " + path;
            outputConsole(error);
            throw error;
        }
        else {
            std::string buffer = Vital::FileSystem::read(path);
            rwBuffer += "\nR\"" + vSignature + "(\n" + buffer + "\n)" + vSignature + "\",";
            outputConsole("Bundled File: " + path);
        }
    }
    rwBuffer += "\n};";
    std::string path = entry + "bundle.h";
    Vital::FileSystem::write(path, rwBuffer);
    outputConsole("Packaged " + name + " successfully!");
}

int main() {
    genPackage("Lua", "Sandbox/lua/module/", Vital::Lua::vModules);
    return 0;
}