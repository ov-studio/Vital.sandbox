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
                std::string ref;
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

    
            // Checkers //
            static bool is_eligible(const std::string& type);
            bool is_loaded(const std::string& name) const;
            bool is_running(const std::string& name) const;


            // Getters //
            static std::string get_resource_base(const std::string& name);
            static std::string get_resource_env(const std::string& name);
            std::vector<const ResourceManifest*> get_all_resources() const;
            const ResourceManifest* get_resource(const std::string& name) const;
            std::vector<ResourceScript> get_scripts(const std::string& name, const std::string& type) const;


            // APIs //
            void scan();
            bool start(const std::string& name);
            bool stop(const std::string& name);
            bool restart(const std::string& name);
            void start_all();
            void stop_all();
    };
}