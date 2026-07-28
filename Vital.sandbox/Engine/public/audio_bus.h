/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio_bus.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Audio Bus Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


///////////////////////////////
// Vital: Engine: Audio_Bus //
///////////////////////////////

namespace Vital::Engine {
    class Audio_Bus {
        protected:
            std::string bus_name;
            int bus_index = -1;


            // Helpers //
            const std::string& create_bus(const std::string& prefix);
            void destroy_bus();
            int resolve_bus() const;
            bool resolve_effect(int id, int& out_idx) const;
        public:
            // Checkerss //
            bool is_effect_enabled(int id) const;

            
            // Getters //
            godot::Ref<godot::AudioEffect> get_effect(int id) const;
            int get_effect_count() const;


            // Setters //
            bool set_effect_enabled(int id, bool enabled);

            
            // Misc //
            bool add_effect(const godot::Ref<godot::AudioEffect>& effect);
            bool remove_effect(int id);
    };
}
#endif