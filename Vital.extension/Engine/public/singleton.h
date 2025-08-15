/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: singleton.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/System/public/vital.h>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/environment.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>


//////////////////////////////
// Vital: Godot: Singleton //
//////////////////////////////

namespace Vital::Godot {
	class Singleton : public godot::Node {
		GDCLASS(Singleton, godot::Node)
		protected:
			static void _bind_methods();
		public:
			Singleton();
			~Singleton() override = default;
			void _process(double delta);
	};	
}