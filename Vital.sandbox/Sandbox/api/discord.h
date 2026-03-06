/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: api: discord.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Discord APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/machine.h>
#include <Vital.sandbox/System/discord.h>


///////////////////////////////////
// Vital: Sandbox: API: Discord //
///////////////////////////////////

namespace Vital::Sandbox::API {
    struct Discord : vm_module {
        inline static const std::string base_name = "discord";

        static void bind(Machine* vm) {
            #if defined(Vital_SDK_Client)
            API::bind(vm, {base_name}, "is_connected", [](auto vm) -> int {
                vm -> push_bool(Vital::System::Discord::get_singleton() -> is_connected());
                return 1;
            });

            API::bind(vm, {base_name}, "set_application_id", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto id = static_cast<uint64_t>(vm -> get_int(1));
                bool authenticate = false;
                bool force_reauth = false;
                if ((vm -> get_arg_count() >= 2) && (vm -> is_bool(2))) authenticate = vm -> get_bool(2);
                if ((vm -> get_arg_count() >= 3) && (vm -> is_bool(3))) force_reauth = vm -> get_bool(3);
                vm -> push_bool(Vital::System::Discord::get_singleton() -> set_application_id(id, authenticate, force_reauth));
                return 1;
            });

            API::bind(vm, {base_name}, "set_activity", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 2) || (!vm -> is_string(1)) || (!vm -> is_string(2))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                Vital::System::Discord::Activity data;
                data.state = vm -> get_string(1);
                data.details = vm -> get_string(2);
                vm -> push_bool(Vital::System::Discord::get_singleton() -> set_activity(data));
                return 1;
            });

            API::bind(vm, {base_name}, "reset_activity", [](auto vm) -> int {
                vm -> push_bool(Vital::System::Discord::get_singleton() -> reset_activity());
                return 1;
            });

            API::bind(vm, {base_name}, "set_state", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto state = vm -> get_string(1);
                vm -> push_bool(Vital::System::Discord::get_singleton() -> set_state(state));
                return 1;
            });

            API::bind(vm, {base_name}, "set_details", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto details = vm -> get_string(1);
                vm -> push_bool(Vital::System::Discord::get_singleton() -> set_details(details));
                return 1;
            });

            API::bind(vm, {base_name}, "set_largeimage", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto key = vm -> get_string(1);
                std::string text = "";
                if ((vm -> get_arg_count() >= 2) && (vm -> is_string(2))) text = vm -> get_string(2);
                vm -> push_bool(Vital::System::Discord::get_singleton() -> set_largeimage(key, text));
                return 1;
            });

            API::bind(vm, {base_name}, "set_smallimage", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_string(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto key = vm -> get_string(1);
                std::string text = "";
                if ((vm -> get_arg_count() >= 2) && (vm -> is_string(2))) text = vm -> get_string(2);
                vm -> push_bool(Vital::System::Discord::get_singleton() -> set_smallimage(key, text));
                return 1;
            });

            API::bind(vm, {base_name}, "set_timestamps", [](auto vm) -> int {
                if ((vm -> get_arg_count() < 1) || (!vm -> is_number(1))) throw Vital::Log::fetch("invalid-arguments", Vital::Log::Type::Error);
                auto start_at = static_cast<int64_t>(vm -> get_int(1));
                int64_t end_at = 0;
                if ((vm -> get_arg_count() >= 2) && (vm -> is_number(2))) end_at = static_cast<int64_t>(vm -> get_int(2));
                vm -> push_bool(Vital::System::Discord::get_singleton() -> set_timestamps(start_at, end_at));
                return 1;
            });

            API::bind(vm, {base_name, "discord"}, "get_user_id", [](auto vm) -> int {
                if (!Vital::System::Discord::get_singleton() -> is_connected()) vm -> push_bool(false);
                else vm -> push_number(static_cast<double>(Vital::System::Discord::get_singleton() -> get_user_id()));
                return 1;
            });

            API::bind(vm, {base_name, "discord"}, "get_username", [](auto vm) -> int {
                if (!Vital::System::Discord::get_singleton() -> is_connected()) vm -> push_bool(false);
                else vm -> push_string(Vital::System::Discord::get_singleton() -> get_username());
                return 1;
            });
            #endif
        }
    };
}