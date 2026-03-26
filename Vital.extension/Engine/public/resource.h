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
    class ResourceManager {
        protected:
            struct ResourceScript {
                std::string src;
                std::string type;
            };

            struct ResourceManifest {
                std::string folder;
                std::string name;
                std::string author;
                std::string version;
                std::vector<ResourceScript> scripts;
                std::vector<std::string> files;
            };

            inline static ResourceManager* singleton = nullptr;
            inline static const std::unordered_set<std::string> valid_script_types = { "server", "client", "shared" };
            std::vector<ResourceManifest> resources;
            std::unordered_set<std::string> running;
        public:
            // Instantiators //
            ResourceManager() = default;
            ~ResourceManager() = default;


            // Utils //
            static ResourceManager* get_singleton();
            static void free_singleton();


            // Helpers //
            static std::string env_name(const std::string& folder);
            static std::string resource_path(const std::string& folder);

        
            // Checkers //
            static bool is_eligible(const std::string& type);
            bool is_loaded(const std::string& folder) const;
            bool is_running(const std::string& folder) const;


            // APIs //
            void scan();
            const ResourceManifest* get_resource(const std::string& folder) const;
            std::vector<const ResourceManifest*> get_all_resources() const;
            std::vector<ResourceScript> get_scripts(const std::string& folder, const std::string& type) const;


            // Lifecycle //
            bool start(const std::string& folder);
            bool stop(const std::string& folder);
            bool restart(const std::string& folder);
            void start_all();
            void stop_all();
    };
}