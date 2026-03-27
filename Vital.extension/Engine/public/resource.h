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
    // TODO: Improve
    class ResourceManager {
        public:
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

            inline static const std::unordered_set<std::string> valid_types = {
                "shared",
                "server",
                "client"
            };

        protected:
            inline static ResourceManager* singleton = nullptr;
            std::vector<ResourceManifest> resources;
            std::unordered_set<std::string> running;

            #if defined(Vital_SDK_Client)
            // Tracks which asset paths belong to each resource so we can
            // cancel only that resource's downloads when it stops mid-transfer
            std::unordered_map<std::string, std::unordered_set<std::string>> resource_assets;
            // Resources that are pending asset download before scripts can load
            std::unordered_set<std::string> pending;
            #endif

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

            #if defined(Vital_SDK_Client)
            bool is_pending(const std::string& name) const;
            #endif


            // Getters //
            std::vector<const ResourceManifest*> get_all_resources() const;
            const ResourceManifest* get_resource(const std::string& name) const;
            static std::string get_resource_base(const std::string& name);
            static std::string get_resource_env(const std::string& name);
            std::vector<ResourceScript> get_resource_scripts(const std::string& name, const std::string& type) const;


            // APIs //
            void scan();

            #if !defined(Vital_SDK_Client)
            bool start(const std::string& name);
            bool stop(const std::string& name);
            bool restart(const std::string& name);
            void start_all();
            void stop_all();
            #endif

            #if defined(Vital_SDK_Client)
            bool load(const std::string& name);
            bool unload(const std::string& name);
            #endif
    };
}