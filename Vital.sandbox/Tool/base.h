/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: 3.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Base Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/log.h>


////////////////////////
// Vital: Tool: Base //
////////////////////////

namespace Vital::Tool {
    template<typename Derived>
    class Base {
        // Derived must declare:
        // static constexpr const char* Name = "classname";
        // friend class Base<Derived>;

        protected:
            inline static Derived* singleton = nullptr;
            Base() = default;
            virtual ~Base() = default;
        public:
            // Singleton //
            static Derived* get_singleton() {
                if (!singleton) {
                    singleton = memnew(Derived);
                    singleton -> init();
                }
                return singleton;
            }

            static void free_singleton() {
                if (!singleton) return;
                singleton -> teardown();
                memdelete(singleton);
                singleton = nullptr;
            }


            // Managers //
            inline void init() {}
            inline void ready() {}
            inline void teardown() {}
            inline void log(const std::string& mode, const std::string& message) const {
                Tool::print(mode, fmt::format("{}: {}", Derived::Name, message));
            }
    };
}