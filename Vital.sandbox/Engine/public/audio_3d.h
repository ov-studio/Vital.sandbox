/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio_3d.h
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
#include <Vital.sandbox/Engine/public/audio_node.h>


//////////////////////////////
// Vital: Engine: Audio_3D //
//////////////////////////////

namespace Vital::Engine {
    struct Audio_3D_Traits {
        using Base = godot::Node3D;
        using Player = godot::AudioStreamPlayer3D;
        static constexpr const char* bus = "audio_3d";
        static constexpr bool unspawn = true;
    };
    
    using Audio_3D = Audio_Node<Audio_3D_Traits>;
}
#endif
