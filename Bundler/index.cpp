#pragma once
#include <Sandbox/lua/public/index.h>
#include <System/public/filesystem.h>

#include <Sandbox/lua/module/bundle.h>

const std::string vSignature = "12a816e2d0469bf62c87e82ac58d58134b7ca9dd";
void outputConsole(std::string message) { std::cout << "\nVital.bundler | " << message; }
void genLuaPackage() {
    std::string entryPoint = "Sandbox/lua/module/";
    std::string rwBuffer = "std::vector<std::string> buffer = {";
    outputConsole("Packaging Lua...");
    for (auto i : Vital::Lua::vModules) {
        std::string path = entryPoint + i;
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
    std::string path = entryPoint + "bundle.h";
    Vital::FileSystem::write(path, rwBuffer);
    outputConsole("Packaged Lua successfully!");
}

int main() {
    genLuaPackage();

    /*
    std::string path = "../";
    Vital::FileSystem::resolve(path);
    path += ".gitignore";
    std::cout << "\nCurrent Tick: " << Vital::getApplicationTick();
    std::cout << "\nResolved Path: " << path;
    std::cout << "\nIs File Existing: " << Vital::FileSystem::exists(path) << "\n";

    Vital::Lua::API::onErrorHandle([](std::string& error) -> void {
        std::cout << "\nR-ERROR | " << error;
    });
    Vital::Lua::API::boot();
    Vital::Lua::vital_vm* test = new Vital::Lua::create();

    //test->registerFunction("me", reinterpret_cast<Vital::Lua::vital_exec>(mefunc));
    std::string testbuffer = "print('System Tick: '..tostring(engine.getSystemTick()))";
    test->loadString(testbuffer);
    testbuffer = "print('Application Tick: '..tostring(engine.getApplicationTick()))";
    test->loadString(testbuffer);
    testbuffer = "print('Current Path: '..file.resolve('../'))";
    test->loadString(testbuffer);
    */
    return 0;
}