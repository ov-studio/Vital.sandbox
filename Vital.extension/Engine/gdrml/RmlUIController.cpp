#include "RmlUIController.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/gd_script.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <RmlUi/Lua.h>

using namespace gdrml;

namespace {
RmlUIController* instance = nullptr;
}

void RmlUIController::_bind_methods()
{
}

RmlUIController* RmlUIController::Instance()
{
	return instance;
}

RmlUIController::RmlUIController()
{
}

RmlUIController::~RmlUIController()
{
}

void RmlUIController::_ready()
{

}

void RmlUIController::_enter_tree()
{
	instance = this;

	// always process
	set_process_mode(PROCESS_MODE_ALWAYS);

	set_name("RmlUIController");
	Engine::get_singleton()->register_singleton("RmlUIController", this);

	// grab our interfaces and apply them
	Rml::SetRenderInterface(&m_RenderInterface);
	Rml::SetSystemInterface(&m_SystemInterface);
	Rml::SetFileInterface(&m_FileInterface);

	if(!Rml::Initialise()){
		godot::UtilityFunctions::print("RmlUIController: Rml::Initialise() failed!");
		return;
	}
	
	//Rml::Lua::Initialise();

	// Load fonts
	Rml::LoadFontFace("res://fonts/Roboto-Regular.ttf");

	Rml::Factory::RegisterEventListenerInstancer(&m_EventListenerInstancer);
}

void RmlUIController::_exit_tree()
{
	Rml::Shutdown();
}

void RmlUIController::_process(double delta)
{
}

void RmlUIController::_notification(int p_what)
{
}
