#pragma once

#include <memory>
#include <vector>

#include "GDScriptEventListener.hpp"

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/input_event.hpp>

using namespace godot;

namespace Rml
{

class Context;
class ElementDocument;

}

namespace gdrml
{

class RmlNode : public Control
{
	GDCLASS(RmlNode, Control);

protected:
	static void _bind_methods();

public:
	RmlNode();
	~RmlNode();

	void set_document_path(const String newPath);
	String get_document_path() const;

	void connect_event(const String elementId, const String eventName, Callable func);

	void _ready() override;
	void _enter_tree() override;
	void _exit_tree() override;
	void _process(double deltaTime) override;
	void _draw() override;
	void _gui_input(const Ref<InputEvent>& event) override;

private:
	String m_DocumentPath;
	Rml::Context* m_Context;
	Rml::ElementDocument* m_Document;
	std::vector<GDScriptEventListener*> m_Listeners;
};

}
