/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: API: discord.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Discord APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(Vital_SDK_Client)
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Manager/public/discord.h>


//////////////////////////
// Vital: API: Discord //
//////////////////////////

// TODO: Update API
namespace Vital::Sandbox::API {
    struct Discord : vm_module {
        inline static const std::string base_name = "discord";

        static void bind(Machine* vm) {
            API::bind(vm, {base_name}, "is_connected", [](auto vm) -> int {
                vm -> push_value(Manager::Discord::get_singleton() -> is_connected());
                return 1;
            });

            API::bind(vm, {base_name}, "set_application_id", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto id = static_cast<uint64_t>(std::stoull(vm -> get_string(1)));
                bool authenticate = vm -> is_bool(2) ? vm -> get_bool(2) : false;
                bool force_reauth = vm -> is_bool(3) ? vm -> get_bool(3) : false;
                vm -> push_value(Manager::Discord::get_singleton() -> set_application_id(id, authenticate, force_reauth));
                return 1;
            });

            API::bind(vm, {base_name}, "reset_application", [](auto vm) -> int {
                vm -> push_value(Manager::Discord::get_singleton() -> reset_application());
                return 1;
            });

            API::bind(vm, {base_name}, "set_state", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_string(1);
                vm -> push_value(Manager::Discord::get_singleton() -> set_state(state));
                return 1;
            });

            API::bind(vm, {base_name}, "set_details", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto details = vm -> get_string(1);
                vm -> push_value(Manager::Discord::get_singleton() -> set_details(details));
                return 1;
            });

            API::bind(vm, {base_name}, "set_largeimage", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto key = vm -> get_string(1);
                std::string text = vm -> is_string(2) ? vm -> get_string(2) : "";
                if (!text.empty()) vm -> push_value(Manager::Discord::get_singleton() -> set_largeimage(key, text));
                else vm -> push_value(Manager::Discord::get_singleton() -> set_largeimage(key));
                return 1;
            });

            API::bind(vm, {base_name}, "set_smallimage", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto key = vm -> get_string(1);
                std::string text = vm -> is_string(2) ? vm -> get_string(2) : "";
                if (!text.empty()) vm -> push_value(Manager::Discord::get_singleton() -> set_smallimage(key, text));
                else vm -> push_value(Manager::Discord::get_singleton() -> set_smallimage(key));
                return 1;
            });

            API::bind(vm, {base_name}, "set_timestamps", [](auto vm) -> int {
                if ((vm -> get_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto start_at = static_cast<int64_t>(vm -> get_int(1));
                int64_t end_at = vm -> is_number(2) ? static_cast<int64_t>(vm -> get_int(2)) : 0;
                vm -> push_value(Manager::Discord::get_singleton() -> set_timestamps(start_at, end_at));
                return 1;
            });

            API::bind(vm, {base_name}, "get_userid", [](auto vm) -> int {
                if (!Manager::Discord::get_singleton() -> is_connected()) vm -> push_value(false);
                else vm -> push_value(std::to_string(Manager::Discord::get_singleton() -> get_userid()));
                return 1;
            });

            API::bind(vm, {base_name}, "get_username", [](auto vm) -> int {
                if (!Manager::Discord::get_singleton() -> is_connected()) vm -> push_value(false);
                else vm -> push_value(Manager::Discord::get_singleton() -> get_username());
                return 1;
            });
        }
    };
}
#else
namespace Vital::Sandbox::API {
    struct Discord : vm_module {};
}
#endif