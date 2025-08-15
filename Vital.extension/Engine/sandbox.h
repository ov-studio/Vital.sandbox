#pragma once
#include "godot_cpp/classes/node.hpp"

class Sandbox : public godot::Node {
	GDCLASS(Sandbox, godot::Node)

	protected:
		static void _bind_methods();

	public:
		Sandbox();
		~Sandbox() override = default;
		void _process(double delta);
};
