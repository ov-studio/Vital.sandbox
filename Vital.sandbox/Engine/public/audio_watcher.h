/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: audio_watcher.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 02/11/2026
     Desc: Audio Watcher Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>


//////////////////////////////////
// Vital: Engine: Audio_Watcher //
//////////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    class Audio_Watcher : public godot::Node {
        GDCLASS(Audio_Watcher, godot::Node)
        private:
            std::function<bool()> playing_resolver;
            std::function<void()> start_handler;
            std::function<void()> finish_handler;
            bool was_playing = false;

            // Helpers //
            void notify_start();
            void notify_finish();


            // Instantiators //
            Audio_Watcher() = default;
            ~Audio_Watcher() override = default;
            static void _bind_methods() {
                godot::ClassDB::bind_method(godot::D_METHOD("_on_finished"), &Audio_Watcher::on_finished);
            }
        public:
            // Hooks //
            void _process(double delta) override;


            // Events //
            void on_finished();
            void set_start_handler(std::function<void()> handler);
            void set_finish_handler(std::function<void()> handler);


            // Managers //
            static Audio_Watcher* create(godot::Node* player, std::function<bool()> playing_resolver);
            void detach();
    };
}
#endif
