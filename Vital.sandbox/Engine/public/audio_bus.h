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
        public:
            enum class Format {
                OGG,
                WAV,
                MP3,
                UNKNOWN
            };

            inline static const std::vector<Tool::Format::Descriptor<Format>> format_registry = {
                { Format::OGG, "ogg", { 0x4F, 0x67, 0x67, 0x53 } },
                { Format::WAV, "wav", { 0x52, 0x49, 0x46, 0x46 } },
                { Format::MP3, "mp3", { 0x49, 0x44, 0x33 }       }
            };
        protected:
            std::string bus_name;
            int bus_index = -1;
            std::vector<std::string> fx_buffer;


            // Helpers //
            const std::string& create_bus(const std::string& prefix);
            void destroy_bus();
            int resolve_bus() const;
            bool resolve_fx(const std::string& name, int& out_bus_idx, int& out_slot_idx) const;
            static godot::Ref<godot::AudioStream> load_stream_from_buffer(const godot::PackedByteArray& buffer);
            static bool get_stream_loop(const godot::Ref<godot::AudioStream>& stream);
            static void set_stream_loop(const godot::Ref<godot::AudioStream>& stream, bool loop);
        public:
            // Checkers //
            virtual godot::Ref<godot::AudioStream> get_stream() const = 0;
            bool is_looped() const;
            bool is_fx_enabled(const std::string& name) const;


            // Getters //
            godot::Ref<godot::AudioEffect> get_fx(const std::string& name) const;
            std::vector<std::string> list_effects() const;


            // Setters //
            bool set_looped(bool loop);
            bool set_fx_enabled(const std::string& name, bool enabled);


            // Misc //
            bool add_fx(const std::string& name, const godot::Ref<godot::AudioEffect>& effect);
            bool remove_fx(const std::string& name);
    };
}
#endif