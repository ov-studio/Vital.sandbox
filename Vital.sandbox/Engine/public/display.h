/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: display.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Display Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


/////////////////////////////
// Vital: Engine: Display //
/////////////////////////////

namespace Vital::Engine {
    class Display : public godot::Object, public Tool::Base<Display> {
        GDCLASS(Display, godot::Object)
        friend class Tool::Base<Display>;
        public:
            static constexpr const char* Name = "Display.engine";
        private:
            // Instantiators //
            Display() = default;
            ~Display() override = default;

            static void _bind_methods() {}
        public:
            // Getters //
            godot::Vector2 get_resolution();
            godot::Ref<godot::ViewportTexture> get_texture();


            // Misc //
            void screenshot(const std::string& base, const std::string& path);
    };
}
#endif
