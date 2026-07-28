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
            int32_t bus_index = -1;

            int32_t resolve_bus_index() const;
            const std::string& create_bus(const std::string& prefix);
            void destroy_bus();
        public:
            bool is_effect_enabled(int32_t effect_index) const;
            godot::Ref<godot::AudioEffect> get_effect(int32_t effect_index) const;
            int32_t get_effect_count() const;
            bool set_effect_enabled(int32_t effect_index, bool enabled);
            bool add_effect(const godot::Ref<godot::AudioEffect>& effect);
            bool remove_effect(int32_t effect_index);
    };
}
#endif