/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: viewtarget.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Viewtarget Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


////////////////////////////////
// Vital: Engine: Viewtarget //
////////////////////////////////

namespace Vital::Engine {
    class Viewtarget : public godot::Object, public Tool::Base<Viewtarget> {
        GDCLASS(Viewtarget, godot::Object)
        friend class Tool::Base<Viewtarget>;
        public:
            static constexpr const char* Name = "Viewtarget.engine";
        private:
            // Instantiators //
            Viewtarget() = default;
            ~Viewtarget() override = default;

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
