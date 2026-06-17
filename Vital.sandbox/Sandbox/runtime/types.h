/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: types.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Types
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <Vital.sandbox/Tool/log.h>
#include <Vital.sandbox/Tool/yaml.h>
#include <Vital.sandbox/Tool/stack.h>
#include <Vital.sandbox/Tool/thread.h>
#include <Vital.sandbox/Tool/timer.h>
#include <Vital.sandbox/Tool/file.h>
#include <Vital.sandbox/Tool/format.h>
#include <Vital.sandbox/Tool/event.h>
#include <Vital.sandbox/Tool/database.h>
#include <Vital.sandbox/Tool/http.h>
#include <Vital.sandbox/Tool/inspect.h>
#include <Vital.sandbox/Tool/crypto.h>
#include <Vital.sandbox/Tool/shrinker.h>
#include <Vital.sandbox/Vendor/lua/lua.hpp>


/////////////////////
// Vital: Sandbox //
/////////////////////

namespace Vital::Sandbox {
    class Machine;
    using vm_state = lua_State;
    using vm_exec = lua_CFunction;
    using vm_machines = std::unordered_map<vm_state*, Machine*>;
    using vm_refs = std::unordered_map<std::string, int>;
    using vm_bind = std::function<int(Machine*, const std::string&)>;
    using vm_method = std::function<int(Machine*, void*, const std::string&)>;

    struct vm_api {
        std::function<void(Machine*)> init;
        std::function<void(Machine*)> bind;
        std::function<void(Machine*)> inject;
    };
    using vm_apis = std::vector<vm_api>;
    using vm_env_cleaner = std::function<void(const std::string&)>;
    using vm_env_cleaners = std::vector<vm_env_cleaner>;

    struct vm_error {
        std::string detail;
        std::string partial;
        vm_error(const std::string& detail, const std::string& partial) : detail(detail), partial(partial) {}
    };

    struct vm_args {
        private:
            int arg_offset = 0;

            inline void throw_error(int index, const std::string& reason = "") const {
                int display_index = index - arg_offset;
                const std::string arg = (index - 1 - arg_offset) < (int)arguments.size() ? arguments[index - 1 - arg_offset] : std::to_string(display_index);
                const std::string partial = fmt::format("bad argument #{} '{}' {}", display_index, arg, reason.empty() ? "" : fmt::format("({})", reason));
                const std::string detail = fmt::format("invalid argument\n> Syntax: `{}`\n> Reason: {}", syntax, partial);
                throw vm_error(detail, partial);
            }
        public:
            Machine* vm;
            std::string syntax;
            std::vector<std::string> arguments;

            inline vm_args(Machine* vm, const std::string& syntax) : vm_args(vm, syntax, "") {}
            inline vm_args(Machine* vm, const std::string& id, const std::string& args, bool is_method = false) : vm(vm), syntax(id + args), arg_offset(is_method ? 1 : 0) {
                auto start = syntax.find('(');
                auto end = syntax.find(')');
                if (start == std::string::npos || end == std::string::npos) return;
                std::stringstream ss(syntax.substr(start + 1, end - start - 1));
                std::string token;
                while (std::getline(ss, token, ',')) {
                    token.erase(0, token.find_first_not_of(" \t"));
                    token.erase(token.find_last_not_of(" \t") + 1);
                    if (!token.empty()) arguments.push_back(token);
                }
            }

            template<typename F>
            inline vm_args& require(int index, F&& check) {
                if ((vm -> get_count() < index) || !std::invoke(std::forward<F>(check), vm, index)) throw_error(index);
                return *this;
            }

            template<typename F>
            inline vm_args& optional(int index, F&& check) {
                if ((vm -> get_count() >= index) && !vm -> is_nil(index) && !std::invoke(std::forward<F>(check), vm, index)) throw_error(index);
                return *this;
            }

            template<typename F>
            inline vm_args& validate(int index, F&& check, const std::string& reason = "out of range") {
                if (!std::invoke(std::forward<F>(check), vm, index)) throw_error(index, reason);
                return *this;
            }

            template<typename E>
            inline vm_args& validate_enum(int index, E min, E max) {
                return validate(index, [min, max](Machine* vm, int index) {
                    auto value = vm -> get_int(index);
                    return value >= static_cast<int>(min) && value <= static_cast<int>(max);
                }, fmt::format("out of range [{} - {}]", static_cast<int>(min), static_cast<int>(max)));
            }
    };

    template<typename T>
    struct vm_registry {
        std::mutex mutex;
        std::unordered_map<int, std::shared_ptr<T>> buffer;
        std::atomic<int> next_id { 1 };
    };
}