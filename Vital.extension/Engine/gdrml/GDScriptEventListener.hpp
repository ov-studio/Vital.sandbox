#pragma once

#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <RmlUi/Core/EventListener.h>

using namespace godot;

namespace gdrml
{

class GDScriptEventListener : public Rml::EventListener
{
public:
	GDScriptEventListener(Rml::Element* element, const Rml::String& code);
	GDScriptEventListener(Rml::Element* element, Callable* func);
	~GDScriptEventListener();

	void ProcessEvent(Rml::Event& event) override;
	void OnDetach(Rml::Element*) override;

private:
	// The node that we are attached to.
	Callable* m_Callable;
	Rml::Element* m_Element;
};

}