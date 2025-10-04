/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: index.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Engine Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/System/public/vital.h>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/texture.hpp>
#include <godot_cpp/classes/environment.hpp>
#include <godot_cpp/classes/world_environment.hpp>
#include <godot_cpp/variant/utility_functions.hpp>


///////////////////////////
// Vital: Godot: Engine //
///////////////////////////

namespace Vital::Godot {}
namespace Vital::Godot::Sandbox {}
namespace Vital::Godot::Engine {
	class Singleton : public godot::Node {
		GDCLASS(Singleton, godot::Node)
		protected:
			static void _bind_methods();
		public:
			// Instantiators //
			Singleton();
			~Singleton() override = default;			
			void _process(double delta);


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
				for (int i = 0; i < node -> get_child_count(); ++i) {
					if (result.size() < limit) fetch_nodes_by_type(node -> get_child(i), result);
					else break;
				}
			}
	};	
}