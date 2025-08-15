/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: sandbox.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/node.hpp>


//////////////
// Sandbox //
//////////////

class Sandbox : public godot::Node {
	GDCLASS(Sandbox, godot::Node)

	protected:
		static void _bind_methods();

	public:
		Sandbox();
		~Sandbox() override = default;
		void _process(double delta);
};
