#pragma once

#include <memory>

#include <RmlUi/Core.h>
#include <godot_cpp/classes/node.hpp>

#include "GDRenderInterface.hpp"
#include "GDSystemInterface.hpp"
#include "GDFileInterface.hpp"
#include "GDScriptEventListenerInstancer.hpp"

using namespace godot;

namespace gdrml {

class RmlUIController : public Node
{
	GDCLASS(RmlUIController, Node);

protected:
	static void _bind_methods();

public:
	static RmlUIController* Instance();

	RmlUIController();
	~RmlUIController();

	void _ready() override;
	void _enter_tree() override;
	void _exit_tree() override;
	void _process(double delta) override;
	void _notification(int p_what);

private:
	GDScriptEventListenerInstancer m_EventListenerInstancer;
	GDRenderInterface m_RenderInterface;
	GDSystemInterface m_SystemInterface;
	GDFileInterface m_FileInterface;
};

}
