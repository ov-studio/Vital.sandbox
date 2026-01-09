/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: private: event.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Event System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/System/public/event.h>


/////////////////////////
// Vital: Tool: Event //
/////////////////////////

namespace Vital::System::Event {
    Vital::Tool::Event::ID vsdk_id = 0;
    std::map<std::string, std::map<Vital::Tool::Event::ID, Vital::Tool::Event::Handler>> vsdk_events;

    bool isEvent(const std::string& identifier) {
        return vsdk_events.find(identifier) != vsdk_events.end();
    }

    Vital::Tool::Event::Handle bind(const std::string& identifier, Vital::Tool::Event::Handler exec) {
        if (!isEvent(identifier)) vsdk_events.emplace(identifier, std::map<Vital::Tool::Event::ID, Vital::Tool::Event::Handler> {});
        vsdk_id++;
        vsdk_events.at(identifier).emplace(vsdk_id, exec);
        auto id = vsdk_id;
        return {
            [=]() -> void {
                vsdk_events.at(identifier).erase(id);
                if (vsdk_events.at(identifier).empty()) vsdk_events.erase(identifier);
            }
        };
    }

    bool emit(const std::string& identifier, Vital::Tool::Stack arguments) {
        if (isEvent(identifier)) {
            for (const auto i : vsdk_events.at(identifier)) {
                (i.second)(arguments); 
            }
        }
        return true;
    }
}
