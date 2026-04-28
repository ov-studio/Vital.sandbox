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
            inline static const std::unordered_set<std::string> Types = {"shared", "server", "client"};
    
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
        protected:
            inline static Resource* singleton = nullptr;
        private:
            mutable std::mutex mutex;
            std::vector<Manifest> resources;
            std::unordered_set<std::string> running;
            #if defined(Vital_SDK_Client)
            std::unordered_map<std::string, std::unordered_set<std::string>> resource_assets;
            std::unordered_set<std::string> pending;
            #endif


            // Helpers //
            std::string chunk_name(const std::string& resource, const std::string& src) const;
            Tool::Stack pack_manifest(const Manifest& manifest) const;
            void unpack_manifest(const Tool::Stack& args, std::vector<Script>& scripts, std::vector<std::string>& files) const;
            bool validate_scripts(const std::string& name, std::vector<std::pair<std::string, std::string>>& sources) const;
            void execute_scripts(const std::string& name, std::vector<std::pair<std::string, std::string>>& sources);
            #if !defined(Vital_SDK_Client)
            bool parse_manifest(Manifest& resource, Tool::YAML& manifest, const std::string& base, std::vector<std::string>& errors);
            Tool::Stack build_packet(const std::string& type, const std::string& name, const Manifest* manifest = nullptr) const;
            #endif


            // Facilitators //
            bool is_loaded_unsafe(const std::string& name) const;
            bool is_running_unsafe(const std::string& name) const;
            const Manifest* get_resource_unsafe(const std::string& name) const;
            std::vector<const Manifest*> get_all_resources_unsafe() const;
            #if defined(Vital_SDK_Client)
            bool is_pending_unsafe(const std::string& name) const;
            #endif
        public:
            // Instantiators //
            Resource() = default;
            ~Resource() = default;


            // Utils //
            static Resource* get_singleton();
            static void free_singleton();


            // Managers //
            void log(const std::string& mode, const std::string& message) const;
            void ready();
            #if !defined(Vital_SDK_Client)
            void sync(int peer_id) const;
            #endif


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
            bool start(std::string name);
            bool stop(std::string name);
            #if !defined(Vital_SDK_Client)
            void scan();
            bool restart(std::string name);
            void start_all();
            void stop_all();
            void restart_all();
            #else
            bool load(std::string name, const std::vector<Script>& scripts, const std::vector<std::string>& files);
            #endif
            void execute_resource(std::string name);


            // Shared //
            // TODO: To be removed/moved into model.cpp later
            void queue_spawn(const std::string& name, int authority_peer);
            void flush_spawn_queue(const std::string& loaded_name);
    };
}
