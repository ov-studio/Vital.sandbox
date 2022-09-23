#include <iostream>
#include <System/public/vital.h>
#include <System/public/filesystem.h>
#include <Sandbox/lua/public/api.h>

int main() {
    std::string path = "../";
    Vital::FileSystem::resolve(path);
    path += ".gitignore";
    std::cout << "\nCurrent Tick: " << Vital::getApplicationTick();
    std::cout << "\nResolved Path: " << path;
    std::cout << "\nIs File Existing: " << Vital::FileSystem::exists(path);
    Vital::Lua::API::onErrorHandle([](std::string& error) -> void {
        std::cout << "\nR-ERROR | " << error;
    });
    Vital::Lua::API::boot();
    Vital::Lua::vital_vm* test = new Vital::Lua::create();
    std::string testbuffer = "print('hello')";
    test -> loadString(testbuffer);
    return 0;
}