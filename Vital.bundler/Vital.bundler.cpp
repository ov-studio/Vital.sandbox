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
    //test->registerFunction("me", reinterpret_cast<Vital::Lua::vital_exec>(mefunc));
    std::string testbuffer = "print('hello') print('Table: '..tostring(table)) print('File: '..tostring(file)) print('Tick: '..tostring(engine.getApplicationTick()))";
    //std::string testbuffer = "print('me: '..tostring(me))";
    test -> loadString(testbuffer);
    return 0;
}