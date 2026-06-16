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

            enum class State {
                Loaded,
                Running
            };

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
                std::vector<std::string> models;
                std::unordered_map<std::string, std::string> script_hashes;
                std::unordered_map<std::string, std::string> file_hashes;
            };
        protected:
            inline static Resource* singleton = nullptr;
        private:
            mutable std::mutex mutex;
            std::vector<Manifest> resources;
            std::unordered_set<std::string> running;
            #if !defined(VSDK_Client)
            std::vector<std::string> resource_order;
            #endif

            struct Internal {
                // Helpers //
                static std::string chunk_name(const std::string& resource, const std::string& src);
                static Tool::Stack pack_manifest(const Manifest& manifest);
                static void unpack_manifest(const Tool::Stack& args, std::vector<Script>& scripts, std::vector<std::string>& files, std::vector<std::string>& models);
                static bool validate_scripts(const std::string& name, std::vector<std::pair<std::string, std::string>>& sources);
                static void execute_scripts(const std::string& name, std::vector<std::pair<std::string, std::string>>& sources);
                static void load_models(const std::string& name);
                static void execute_resource(std::string name);
                #if defined(VSDK_Client)
                static bool register_resource(std::string name, const std::vector<Script>& scripts, const std::vector<std::string>& files, const std::vector<std::string>& models);
                #else
                static bool parse_manifest(Manifest& resource, Tool::YAML& manifest, const std::string& base, std::vector<std::string>& errors);
                static Tool::Stack build_packet(const std::string& event, const std::string& name, const Manifest* manifest = nullptr);
                #endif


                // Checkers //
                static bool is_loaded(const std::string& name);
                static bool is_running(const std::string& name);
                #if defined(VSDK_Client)
                static bool is_pending(const std::string& name);
                #endif


                // Getters //
                static const Manifest* get_resource(const std::string& name);
                static std::vector<const Manifest*> get_all_resources();
                static std::vector<const Manifest*> get_resources(State type);


                // APIs //
                static bool start(std::string name);
                static bool stop(std::string name);
                static void stop_all();
                #if !defined(VSDK_Client)
                static void scan();
                static bool restart(std::string name);
                static void start_all();
                static void restart_all();
                #endif
            };
        public:
            // Instantiators //
            Resource() = default;
            ~Resource() = default;


            // Singleton //
            static Resource* get_singleton();
            static void free_singleton();


            // Managers //
            void log(const std::string& mode, const std::string& message) const;
            void ready();
            #if !defined(VSDK_Client)
            void sync(int peer_id) const;
            #endif


            // Checkers //
            static bool is_name(const std::string& name);
            static bool is_type(const std::string& type);
            bool is_loaded(const std::string& name) const;
            bool is_running(const std::string& name) const;
            #if defined(VSDK_Client)
            bool is_pending(const std::string& name) const;
            #endif


            // Getters //
            std::vector<const Manifest*> get_all_resources() const;
            const Manifest* get_resource(const std::string& name) const;
            static std::string get_resource_base(const std::string& name, bool require_running = false);
            static std::string get_resource_from_vm(Vital::Sandbox::Machine* vm);
            std::vector<const Manifest*> get_resources(State type) const;
            int get_resource_count(State type) const;


            // APIs //
            bool start(std::string name);
            bool stop(std::string name);
            void stop_all();
            #if !defined(VSDK_Client)
            void scan();
            bool restart(std::string name);
            void start_all();
            void restart_all();
            #endif
    };
}