/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: resource.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Resource Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/index.h>


//////////////////////////////
// Vital: Engine: Resource //
//////////////////////////////

namespace Vital::Engine {
    struct ResourceScript {
        std::string src;
        std::string type; // "server" | "client" | "shared"
    };

    struct ResourceManifest {
        std::string folder;  // unique folder name — used as resource ID
        std::string name;    // freeform display name from manifest (optional)
        std::string author;
        std::string version;
        std::vector<ResourceScript> scripts;
        std::vector<std::string>   files;
    };

    class ResourceManager {
        protected:
            inline static ResourceManager* singleton = nullptr;
            std::vector<ResourceManifest> resources;

        public:
            // Instantiators //
            ResourceManager() = default;
            ~ResourceManager() = default;


            // Utils //
            static ResourceManager* get_singleton();
            static void free_singleton();


            // APIs //
            void scan();
            bool is_loaded(const std::string& folder) const;
            const ResourceManifest* get_resource(const std::string& folder) const;
            std::vector<const ResourceManifest*> get_all_resources() const;
            std::vector<ResourceScript> get_scripts(const std::string& folder, const std::string& type) const;
    };
}