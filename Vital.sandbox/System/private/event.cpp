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
    Vital::Type::Event::HandleID handleID = 0;
    std::map<std::string, std::map<Vital::Type::Event::HandleID, Vital::Type::Event::Handler>> instance;
    bool isInstance(const std::string& identifier) { return instance.find(identifier) != instance.end(); }

    Vital::Type::Event::Handle bind(const std::string& identifier, Vital::Type::Event::Handler exec) {
        if (!isInstance(identifier)) instance.emplace(identifier, std::map<Vital::Type::Event::HandleID, Vital::Type::Event::Handler> {});
        handleID++;
        instance.at(identifier).emplace(handleID, exec);
        auto __handleID = handleID;
        return {
            [=]() -> void {
                instance.at(identifier).erase(__handleID);
                if (instance.at(identifier).empty()) instance.erase(identifier);
            }
        };
    }
    bool emit(const std::string& identifier, Vital::Type::Event::Arguments arguments) {
        if (isInstance(identifier)) {
            for (const auto i : instance.at(identifier)) {
                (i.second)(arguments); 
            }
        }
        return true;
    }
}
