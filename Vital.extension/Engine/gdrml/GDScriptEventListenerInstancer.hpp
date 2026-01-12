#pragma once

#include <RmlUi/Core/EventListenerInstancer.h>
#include <godot_cpp/classes/node.hpp>

using namespace godot;

namespace gdrml
{

class GDScriptEventListenerInstancer : public Rml::EventListenerInstancer
{
public:
	GDScriptEventListenerInstancer();
	~GDScriptEventListenerInstancer();

	Rml::EventListener* InstanceEventListener(const Rml::String& value, Rml::Element* element) override;
};

}
