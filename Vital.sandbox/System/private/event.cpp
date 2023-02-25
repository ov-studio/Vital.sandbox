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
    std::map<std::string, std::vector<Vital::Type::Event::Handler>> instance;
    bool isInstance(const std::string& identifier) { return instance.find(identifier) != instance.end(); }

    bool bind(const std::string& identifier, Vital::Type::Event::Handler exec) {
        if (!isInstance(identifier)) instance.emplace(identifier, std::vector<Vital::Type::Event::Handler> {});
        instance.at(identifier).push_back(exec);
        return true;
    }
    bool unbind(const std::string& identifier, Vital::Type::Event::Handler exec) {
        if (isInstance(identifier)) {
            for (int i = 0; instance.at(identifier).size(); i++) {
                if (&instance.at(identifier).at(i) == &exec) {
                    std::cout << "removed from index: " << i;
                    instance.at(identifier).erase(instance.at(identifier).begin() + i - 1);
                    break;
                }
            }
            if (instance.at(identifier).empty()) instance.erase(identifier);
        }
        return true;
    }
    bool emit(const std::string& identifier, Vital::Type::Event::Arguments arguments) {
        if (isInstance(identifier)) {
            for (const auto& i : instance.at(identifier)) {
                i(arguments); 
            }
        }
        return true;
    }
}