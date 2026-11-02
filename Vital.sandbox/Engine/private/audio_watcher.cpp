/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio_watcher.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 02/11/2026
     Desc: Audio Watcher Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/audio_watcher.h>


//////////////////////////////////
// Vital: Engine: Audio_Watcher //
//////////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    // Helpers //
    // A playback started: fires "audio:start"
    void Audio_Watcher::notify_start() {
        was_playing = true;
        if (start_handler) start_handler();
    }

    // A playback reached its end: fires "audio:finish"
    void Audio_Watcher::notify_finish() {
        // A sound shorter than a frame can finish before the next poll saw it start: report the start first
        if (!was_playing) notify_start();
        was_playing = false;
        if (finish_handler) finish_handler();
    }


    // Hooks //
    void Audio_Watcher::_process(double) {
        // Playback started by anything: play(), autoplay or the engine itself. Picked up on the rising edge
        const bool playing = playing_resolver && playing_resolver();
        if (playing && !was_playing) notify_start();
        was_playing = playing;
    }


    // Events //
    void Audio_Watcher::on_finished() {
        notify_finish();
    }

    void Audio_Watcher::set_start_handler(std::function<void()> handler) {
        start_handler = std::move(handler);
    }

    void Audio_Watcher::set_finish_handler(std::function<void()> handler) {
        finish_handler = std::move(handler);
    }


    // Managers //
    Audio_Watcher* Audio_Watcher::create(godot::Node* player, std::function<bool()> playing_resolver) {
        auto watcher = memnew(Audio_Watcher);
        watcher -> playing_resolver = std::move(playing_resolver);
        watcher -> set_process(true);

        // Godot's own "finished" signal: only emitted when a playback reaches its end, never by stop() or when looping
        player -> connect("finished", godot::Callable(watcher, "_on_finished"));
        player -> add_child(watcher);
        return watcher;
    }

    void Audio_Watcher::detach() {
        // No signal may fire once the audio is on its way out
        start_handler = nullptr;
        finish_handler = nullptr;
        playing_resolver = nullptr;
    }
}
#endif
