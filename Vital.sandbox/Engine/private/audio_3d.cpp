/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio_3d.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Audio 3D Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/audio_3d.h>


//////////////////////////////
// Vital: Engine: Audio_3D //
//////////////////////////////

namespace Vital::Engine {
    // Instantiators //
    Audio_3D::Audio_3D(const godot::Ref<godot::AudioStream>& stream) {
        this -> stream = stream;
        player = memnew(godot::AudioStreamPlayer3D);
        player -> set_stream(stream);
        player -> set_bus(godot::StringName(Tool::to_godot_string(create_bus("audio_3d"))));
        Engine::Core::get_singleton() -> add_child(player);
    }

    Audio_3D::~Audio_3D() {
        destroy_bus();
        if (!player) return;
        player -> queue_free();
        player = nullptr;
    }


    // Managers //
    Audio_3D* Audio_3D::create(const std::string& base, const std::string& path) {
        return create_from_buffer(Tool::File::read_binary(base, path));
    }

    Audio_3D* Audio_3D::create_from_buffer(const godot::PackedByteArray& buffer) {
        return memnew(Audio_3D(load_stream_from_buffer(buffer)));
    }

    void Audio_3D::destroy() {
        memdelete(this);
    }


    // Getters //
    godot::Ref<godot::AudioStream> Audio_3D::get_stream() const {
        return stream;
    }

    godot::AudioStreamPlayer3D* Audio_3D::get_player() const {
        return player;
    }
}
#endif