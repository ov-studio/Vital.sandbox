/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: resource.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Resource Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>


///////////////////////////////
// Vital: Manager: Resource //
///////////////////////////////

namespace Vital::Manager {
    class Resource {
        public:
            struct Script {
                std::string src;
                std::string type;
            };

            struct Manifest {
                std::string ref;
                std::string name;
                std::string author;
                std::string version;
                std::vector<Script> scripts;
                std::vector<std::string> files;
                std::unordered_map<std::string, std::string> script_hashes;
                std::unordered_map<std::string, std::string> file_hashes;
            };

            inline static const std::unordered_set<std::string> Types = {
                "shared",
                "server",
                "client"
            };
        protected:
            inline static Resource* singleton = nullptr;
            std::vector<Manifest> resources;
            std::unordered_set<std::string> running;
            std::mutex scan_mutex;
            #if defined(Vital_SDK_Client)
            std::unordered_map<std::string, std::unordered_set<std::string>> resource_assets;
            std::unordered_set<std::string> pending;
            #endif
        public:
            // Instantiators //
            Resource() = default;
            ~Resource() = default;


            // Utils //
            static Resource* get_singleton();
            static void free_singleton();


            // Managers //
            void log(const std::string& mode, const std::string& message);


            // Checkers //
            static bool is_name(const std::string& name);
            static bool is_type(const std::string& type);
            bool is_loaded(const std::string& name) const;
            bool is_running(const std::string& name) const;
            #if defined(Vital_SDK_Client)
            bool is_pending(const std::string& name) const;
            #endif


            // Getters //
            std::vector<const Manifest*> get_all_resources() const;
            const Manifest* get_resource(const std::string& name) const;
            static std::string get_resource_from_vm(Vital::Sandbox::Machine* vm);
            static std::string get_resource_base(const std::string& name, bool require_running = false);
            std::vector<Script> get_resource_scripts(const std::string& name, const std::string& type = "") const;


            // APIs //
            void scan();
            void init();
            #if !defined(Vital_SDK_Client)
            bool parse_manifest(Manifest& resource, Tool::YAML& manifest, const std::string& base, std::vector<std::string>& errors);
            bool start(const std::string& name);
            bool stop(const std::string& name);
            bool restart(const std::string& name);
            void start_all();
            void stop_all();
            void restart_all();
            void broadcast_resource_event(const std::string& type, const std::string& name, const Manifest* manifest = nullptr) const;
            #endif
            #if defined(Vital_SDK_Client)
            bool register_remote(const std::string& name, const std::vector<Script>& scripts, const std::vector<std::string>& files);
            void unregister_remote(const std::string& name);
            bool load(const std::string& name);
            bool unload(const std::string& name);
            void execute_scripts(const std::string& name);
            void unpack_manifest_payload(const Tool::Stack& args, std::vector<Script>& scripts, std::vector<std::string>& files) const;
            #endif
    };
}