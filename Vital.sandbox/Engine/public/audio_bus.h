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
            std::vector<std::string> effect_names;


            // Helpers //
            const std::string& create_bus(const std::string& prefix);
            void destroy_bus();
            int resolve_bus() const;
            bool resolve_effect(const std::string& name, int& out_bus_idx, int& out_slot_idx) const;
        public:
            // Checkers //
            bool is_effect_enabled(const std::string& name) const;


            // Getters //
            godot::Ref<godot::AudioEffect> get_effect(const std::string& name) const;
            int get_effect_count() const;
            std::vector<std::string> get_effects() const;


            // Setters //
            bool set_effect_enabled(const std::string& name, bool enabled);


            // Misc //
            bool add_effect(const std::string& name, const godot::Ref<godot::AudioEffect>& effect);
            bool remove_effect(const std::string& name);
    };
}
#endif