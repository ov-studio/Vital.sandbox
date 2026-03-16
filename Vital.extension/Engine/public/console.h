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

#pragma once
#include <Vital.extension/Engine/public/index.h>
#if defined(Vital_SDK_Client)
#include <Vital.extension/Engine/public/webview.h>
#endif


/////////////////////////////
// Vital: Engine: Console //
/////////////////////////////

namespace Vital::Engine {
    class Console : public godot::Control {
        protected:
            inline static Console* singleton = nullptr;
        private:
            #if defined(Vital_SDK_Client)
            Webview* webview = nullptr;
            #endif
        public:
            // Instantiators //
            Console();
            ~Console();


            // Utils //
            static Console* get_singleton();
            static void free_singleton();


            // APIs //
            void print(const std::string& mode, const std::string& message);
            void command(const std::string& input);


            // Events //
            #if defined(Vital_SDK_Client)
            void on_message(godot::String message);
            #endif
    };
}

namespace Vital {
    template<typename... Args>
    inline void print(const std::string& mode, Args&&... args) {
        std::ostringstream oss;
        bool first = true;
        ((oss << (first ? (first = false, "") : " ") << std::forward<Args>(args)), ...);
        Engine::Console::get_singleton() -> print(mode, oss.str());
    }
}