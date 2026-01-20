/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: core.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Core Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/index.h>


///////////////////////////
// Vital: Godot: Engine //
///////////////////////////

namespace Vital::Godot {
	class Core;
	class Core : public godot::Node {
		GDCLASS(Core, godot::Node)
		protected:
			inline static Core* singleton = nullptr;
			inline static godot::WorldEnvironment* environment = nullptr;
			static void _bind_methods() {};
		public:
			// Instantiators //
			Core();
			~Core() override = default;
			void _ready() override;
			void _process(double delta) override;


			// Getters //
			static Core* get_singleton();
			static godot::Node* get_root();
			#if defined(Vital_SDK_Client)
			static godot::RenderingServer* get_rendering_server();
			static godot::Ref<godot::Environment> get_environment();
			static void free_environment();
			#endif

	
			// Utils //
			template <typename T>
			static void fetch_nodes_by_type(godot::Node* node, std::vector<T*> &result, int limit = -1) {
				if (!node || (limit == 0)) return;
				T* typed_node = godot::Object::cast_to<T>(node);
				if (typed_node) result.push_back(typed_node);
				for (int i = 0; i < node -> get_child_count(); i++) {
					if ((limit == -1) || (result.size() < limit)) fetch_nodes_by_type(node -> get_child(i), result, limit);
					else break;
				}
			}
	};
}