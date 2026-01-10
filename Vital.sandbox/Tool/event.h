/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: public: event.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Event Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/System/public/vital.h>


/////////////////////////
// Vital: Tool: Event //
/////////////////////////

namespace Vital::System::Event {
    using EventID = unsigned long;
    using Handler = std::function<void(Vital::Tool::Stack)>;
    using EventMap = std::map<EventID, Handler>;
    using EventPool = std::map<std::string, EventMap>;
    static inline EventID id = 0;
    static inline EventPool pool;

    static inline EventID bind(const std::string& identifier, Handler exec) {
        auto it = pool.find(identifier);
        if (it == pool.end()) pool[identifier] = {};
        pool[identifier].emplace(++id, std::move(exec));
        return id;
    }

    static inline bool unbind(const std::string& identifier, EventID id) {
        auto it = pool.find(identifier);
        if (it == pool.end()) return false;
        it -> second.erase(id);
        if (it -> second.empty()) pool.erase(it);
        return true;
    }

    static inline bool emit(const std::string& identifier, Vital::Tool::Stack arguments = {}) {
        auto it = pool.find(identifier);
        if (it == pool.end()) return false;
        for (const auto& [id, handler] : it -> second) {
            if (handler)
                handler(arguments);
        }
        return true;
    }
}