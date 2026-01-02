/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: singleton.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Singleton Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/index.h>


///////////////////////////
// Vital: Godot: Engine //
///////////////////////////

namespace Vital::Godot::Engine {
	class Canvas;

	class Singleton : public godot::Node {
		GDCLASS(Singleton, godot::Node)
		protected:
			static void _bind_methods() {};
		private:
			Canvas* canvas = nullptr;
		public:
			// Instantiators //
			Singleton();
			~Singleton() override = default;
			void _ready() override;
			void _process(double delta) override;


			// Getters //
			static godot::Node* get_root();
			static godot::ResourceLoader* get_resource_loader();
			static godot::Ref<godot::Environment> get_environment();

	
			// Utils //
			template <typename T>
			static void fetch_nodes_by_type(godot::Node *node, std::vector<T*> &result, int limit = -1) {
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