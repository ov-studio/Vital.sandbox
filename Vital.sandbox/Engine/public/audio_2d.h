g/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio_2d.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Audio 2D Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/audio_node.h>


//////////////////////////////
// Vital: Engine: Audio_2D //
//////////////////////////////

namespace Vital::Engine {
    struct Audio_2D_Traits {
        using Base = godot::Node2D;
        using Player = godot::AudioStreamPlayer2D;
        static constexpr const char* bus = "audio_2d";
        static constexpr bool unspawn = false;
    };
    
    using Audio_2D = Audio_Node<Audio_2D_Traits>;
}
#endif
