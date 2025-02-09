/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: System: private: event.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Event System
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <System/public/event.h>


/////////////////////////
// Vital: Type: Event //
/////////////////////////

namespace Vital::System::Event {
    Vital::Type::Event::ID vsdk_id = 0;
    std::map<std::string, std::map<Vital::Type::Event::ID, Vital::Type::Event::Handler>> vsdk_events;

    bool isInstance(const std::string& identifier) {
        return vsdk_events.find(identifier) != vsdk_events.end();
    }

    Vital::Type::Event::Handle bind(const std::string& identifier, Vital::Type::Event::Handler exec) {
        if (!isInstance(identifier)) vsdk_events.emplace(identifier, std::map<Vital::Type::Event::ID, Vital::Type::Event::Handler> {});
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

    bool emit(const std::string& identifier, Vital::Type::Stack::Instance arguments) {
        if (isInstance(identifier)) {
            for (const auto i : vsdk_events.at(identifier)) {
                (i.second)(arguments); 
            }
        }
        return true;
    }
}
