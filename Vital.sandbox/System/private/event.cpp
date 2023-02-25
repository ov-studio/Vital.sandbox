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
    std::map<std::string, std::map<Vital::Type::Event::Handler*, Vital::Type::Event::Handler>> instance;
    bool isInstance(const std::string& identifier) { return instance.find(identifier) != instance.end(); }

    bool bind(const std::string& identifier, Vital::Type::Event::Handler exec) {
        if (!isInstance(identifier)) instance.emplace(identifier, std::map<Vital::Type::Event::Handler*, Vital::Type::Event::Handler> {});
        instance.at(identifier).emplace(&exec, exec);
        return true;
    }
    bool unbind(const std::string& identifier, Vital::Type::Event::Handler exec) {
        if (isInstance(identifier)) {
            instance.at(identifier).erase(&exec);
            if (instance.at(identifier).empty()) instance.erase(identifier);
        }
        return true;
    }
    bool emit(const std::string& identifier, Vital::Type::Event::Arguments arguments) {
        if (isInstance(identifier)) {
            for (const auto& i : instance.at(identifier)) {
                i.second(); 
            }
        }
        return true;
    }
}
