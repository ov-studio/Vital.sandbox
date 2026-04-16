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
        private:
            std::vector<Manifest> resources;
            std::unordered_set<std::string> running;
            #if !defined(Vital_SDK_Client)
            std::mutex scan_mutex;
            #else
            std::unordered_map<std::string, std::unordered_set<std::string>> resource_assets;
            std::unordered_set<std::string> pending;
            #endif
        private:
            // Helpers //
            std::string chunk_name(const std::string& resource, const std::string& src);
        public:
            // Instantiators //
            Resource() = default;
            ~Resource() = default;


            // Utils //
            static Resource* get_singleton();
            static void free_singleton();


            // Managers //
            void log(const std::string& mode, const std::string& message);
            void unpack_manifest(const Tool::Stack& args, std::vector<Script>& scripts, std::vector<std::string>& files) const;


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
            void scan();
            bool start(std::string name);
            bool stop(std::string name);
            bool restart(std::string name);
            void start_all();
            void stop_all();
            void restart_all();
            Tool::Stack build_packet(const std::string& type, const std::string& name, const Manifest* manifest = nullptr) const;
            void sync_peer(int peer_id) const;
            #endif
            #if defined(Vital_SDK_Client)
            bool load(std::string name, const std::vector<Script>& scripts, const std::vector<std::string>& files);
            bool unload(std::string name);
            void execute_scripts(std::string name);
            #endif


            // Shared //
            void queue_spawn(const std::string& name, int authority_peer);
            void flush_spawn_queue(const std::string& loaded_name);
    };
}