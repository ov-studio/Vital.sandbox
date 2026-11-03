/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio_node.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 03/11/2026
     Desc: Audio Node Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/audio_bus.h>


////////////////////////////////
// Vital: Engine: Audio_Node //
////////////////////////////////

namespace Vital::Engine {
    template<typename Traits>
    class Audio_Node : public Traits::Base, public Engine::Audio_Bus {
        private:
            godot::Ref<godot::AudioStream> stream;
            typename Traits::Player* player = nullptr;


            // Instantiators //
            Audio_Node(const godot::Ref<godot::AudioStream>& stream, bool autoplay) {
                this -> stream = stream;
                player = memnew(typename Traits::Player);
                player -> set_stream(stream);
                player -> set_autoplay(autoplay);
                player -> set_bus(godot::StringName(Tool::to_godot_string(create_bus(Traits::bus))));
                Engine::Core::get_singleton() -> add_child(player);
                create_watcher(player, [this]() { return player -> is_playing(); });
            }

            ~Audio_Node() {
                if constexpr (Traits::unspawn) Tool::Event::emit("entity:unspawn", Tool::Stack({this}));
                destroy_watcher();
                destroy_bus();
                if (!player) return;
                player -> queue_free();
                player = nullptr;
            }
        public:
            // Managers //
            static Audio_Node* create(const std::string& base, const std::string& path, bool autoplay = false) {
                return create_from_buffer(Tool::File::read_binary(base, path), autoplay);
            }

            static Audio_Node* create_from_buffer(const godot::PackedByteArray& buffer, bool autoplay = false) {
                return memnew(Audio_Node(load_stream_from_buffer(buffer), autoplay));
            }

            void destroy() {
                memdelete(this);
            }


            // Getters //
            godot::Ref<godot::AudioStream> get_stream() const {
                return stream;
            }

            typename Traits::Player* get_player() const {
                return player;
            }
    };
}
#endif
