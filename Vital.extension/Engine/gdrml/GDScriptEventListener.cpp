#include "GDScriptEventListener.hpp"

#include <godot_cpp/classes/gd_script.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "Conversion.h"

using namespace gdrml;

GDScriptEventListener::GDScriptEventListener(Rml::Element* element, const Rml::String& code) : Rml::EventListener()
{
	m_Element = element;

	// gdscript inlined
	String gdCode = "static func _eval(): " + Conversion::stringToGodot(code);

	// hopefully this doesn't bite us in the ass?
	GDScript* script = new GDScript();
	script->set_source_code(gdCode);
	script->reload();
	m_Callable = new Callable(script, "_eval");
}

GDScriptEventListener::GDScriptEventListener(Rml::Element* element, Callable* func) : Rml::EventListener()
{
	m_Element = element;
	m_Callable = func;
}

GDScriptEventListener::~GDScriptEventListener()
{
	delete m_Callable;
}

void GDScriptEventListener::ProcessEvent(Rml::Event& event)
{
	m_Callable->call();
	event.StopPropagation();
}

void GDScriptEventListener::OnDetach(Rml::Element*)
{
	// we are attached and held by an element.
	delete this;
}
