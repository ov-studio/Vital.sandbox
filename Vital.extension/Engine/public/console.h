/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: console.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Console Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#if defined(Vital_SDK_Client)
#pragma once
#include <Vital.extension/Engine/public/webview.h>


////////////////////////////
// Vital: Godot: Console //
////////////////////////////

namespace Vital::Godot {
    class Console;
    class Console : public godot::Control {
        protected:
            inline static Console* singleton = nullptr;
        private:
            Webview* webview = nullptr;
        public:
            // Instantiators //
            Console();
            ~Console();


            // Utils //
            static Console* get_singleton();
            static void free_singleton();


            // APIs //
            void print(const std::string& mode, const std::string& message);


            // Events //
            void on_message(godot::String message);
    };
}
#endif

namespace Vital {
    template<typename... Args>
    inline void print(const std::string& mode, Args&&... args) {
        if (mode.empty()) return;
        std::ostringstream oss;
        bool first = true;
        ((oss << (first ? (first = false, "") : " ") << std::forward<Args>(args)), ...);
        const std::string message = oss.str();
        if (message.empty()) return;
        #if defined(Vital_SDK_Client)
            Godot::Console::get_singleton() -> print(mode, message);
        #else
            godot::UtilityFunctions::print(to_godot_string(message));
        #endif
    }
}