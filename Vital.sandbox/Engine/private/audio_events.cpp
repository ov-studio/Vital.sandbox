/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio_events.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 02/11/2026
     Desc: Audio Events Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/audio_events.h>


////////////////////////////////
// Vital: Engine: Audio_Events //
////////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    // Helpers //
    // A playback started: fires "audio:start"
    void Audio_Events::notify_start() {
        was_playing = true;
        if (start_handler) start_handler();
    }

    // A playback reached its end: fires "audio:finish"
    void Audio_Events::notify_finish() {
        // A sound shorter than a frame can finish before the next poll saw it start: report the start first
        if (!was_playing) notify_start();
        was_playing = false;
        if (finish_handler) finish_handler();
    }


    // Hooks //
    void Audio_Events::_process(double) {
        // Playback started by anything: play(), autoplay or the engine itself. Picked up on the rising edge
        const bool playing = playing_resolver && playing_resolver();
        if (playing && !was_playing) notify_start();
        was_playing = playing;
    }


    // Events //
    void Audio_Events::on_finished() {
        notify_finish();
    }

    void Audio_Events::set_start_handler(std::function<void()> handler) {
        start_handler = std::move(handler);
    }

    void Audio_Events::set_finish_handler(std::function<void()> handler) {
        finish_handler = std::move(handler);
    }


    // Managers //
    Audio_Events* Audio_Events::create(godot::Node* player, std::function<bool()> playing_resolver) {
        auto events = memnew(Audio_Events);
        events -> playing_resolver = std::move(playing_resolver);
        events -> set_process(true);

        // Godot's own "finished" signal: only emitted when a playback reaches its end, never by stop() or when looping
        player -> connect("finished", godot::Callable(events, "_on_finished"));
        player -> add_child(events);
        return events;
    }

    void Audio_Events::detach() {
        // No signal may fire once the audio is on its way out
        start_handler = nullptr;
        finish_handler = nullptr;
        playing_resolver = nullptr;
    }
}
#endif
