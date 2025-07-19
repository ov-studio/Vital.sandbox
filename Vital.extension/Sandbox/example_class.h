#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"

using namespace godot;

class ExampleClass : public Node {
	GDCLASS(ExampleClass, Node)

protected:
	static void _bind_methods();

public:
	ExampleClass();
	~ExampleClass() override = default;

	void print_type(const Variant &p_variant) const;
	void _process(double delta);
};
