/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: lua: api: private: file.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: File APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Sandbox/lua/api/public/file.h>
#include <System/public/file.h>


///////////////
// Lua: API //
///////////////

namespace Vital::Sandbox::Lua::API {
    void File::bind(void* instance) {
        API::bind("file", "resolve", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                std::string path = vm -> getString(1);
                Vital::System::File::resolve(path);
                vm -> setString(path);
                return 1;
            });
        });

        API::bind("file", "exists", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                std::string path = vm -> getString(1);
                vm -> setBool(Vital::System::File::exists(path));
                return 1;
            });
        });

        API::bind("file", "size", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                std::string path = vm -> getString(1);
                vm -> setNumber(static_cast<double>(Vital::System::File::size(path)));
                return 1;
            });
        });

        API::bind("file", "delete", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                std::string path = vm -> getString(1);
                vm -> setBool(Vital::System::File::remove(path));
                return 1;
            });
        });

        API::bind("file", "read", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                std::string path = vm -> getString(1);
                auto buffer = Vital::System::File::read(path);
                vm -> setString(buffer);
                return 1;
            });
        });

        API::bind("file", "write", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 2) || (!vm -> isString(1)) || (!vm -> isString(2))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                std::string path = vm -> getString(1);
                std::string buffer = vm -> getString(2);
                vm -> setBool(Vital::System::File::write(path, buffer));
                return 1;
            });
        });

        API::bind("file", "contents", [](auto* ref) -> int {
            auto vm = fetchVM(ref);
            return vm -> execute([&]() -> int {
                if ((vm -> getArgCount() < 1) || (!vm -> isString(1))) throw std::runtime_error(ErrorCode["invalid-arguments"]);
                std::string path = vm -> getString(1);
                bool fetchDirs = vm -> isBool(2) ? vm -> getBool(2) : false;
                vm -> createTable();
                for (const auto& i : Vital::System::File::contents(path, fetchDirs)) {
                    vm -> pushString(i);
                }
                return 1;
            });
        });
    }
}