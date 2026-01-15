/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: console.h
     Author: vStudio
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
        GDCLASS(Console, godot::Control)
        protected:
            static inline Console* singleton = nullptr;
			static void _bind_methods() {};
        private:
            Webview* webview = nullptr;
        public:
            // Instantiators //
            Console();
            ~Console();
            void _notification(int what);


            // Getters //
            static Console* get_singleton();
    };
}
#endif