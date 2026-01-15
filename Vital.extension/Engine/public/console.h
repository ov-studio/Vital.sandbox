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
    class Console : public godot::Node2D {
        protected:
            static inline Console* singleton = nullptr;
        private:
            Webview* webview = nullptr;
        public:
            // Instantiators //
            Console();
            ~Console();


            // Getters //
            static Console* get_singleton();


            // APIs //
            void update(); // TODO: Call it whenever godot window resizes
    };
}
#endif