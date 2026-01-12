#include "RmlNode.hpp"

#include "GDRenderInterface.hpp"
#include "RmlUIController.hpp"
#include "GDScriptEventListener.hpp"
#include "GDScriptEventListenerInstancer.hpp"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>

#include "Conversion.h"

using namespace gdrml;

Rml::Input::KeyIdentifier GDKeyToRml(Key key)
{
	switch (key)
	{
	case KEY_UNKNOWN: return Rml::Input::KI_UNKNOWN;
	case KEY_SPACE: return Rml::Input::KI_SPACE;
	case KEY_ESCAPE: return Rml::Input::KI_ESCAPE;
		// Numbers
	case KEY_0: return Rml::Input::KI_0;
	case KEY_1: return Rml::Input::KI_1;
	case KEY_2: return Rml::Input::KI_2;
	case KEY_3: return Rml::Input::KI_3;
	case KEY_4: return Rml::Input::KI_4;
	case KEY_5: return Rml::Input::KI_5;
	case KEY_6: return Rml::Input::KI_6;
	case KEY_7: return Rml::Input::KI_7;
	case KEY_8: return Rml::Input::KI_8;
	case KEY_9: return Rml::Input::KI_9;
		// Letters
	case KEY_A: return Rml::Input::KI_A;
	case KEY_B: return Rml::Input::KI_B;
	case KEY_C: return Rml::Input::KI_C;
	case KEY_D: return Rml::Input::KI_D;
	case KEY_E: return Rml::Input::KI_E;
	case KEY_F: return Rml::Input::KI_F;
	case KEY_G: return Rml::Input::KI_G;
	case KEY_H: return Rml::Input::KI_H;
	case KEY_I: return Rml::Input::KI_I;
	case KEY_J: return Rml::Input::KI_J;
	case KEY_K: return Rml::Input::KI_K;
	case KEY_L: return Rml::Input::KI_L;
	case KEY_M: return Rml::Input::KI_M;
	case KEY_N: return Rml::Input::KI_N;
	case KEY_O: return Rml::Input::KI_O;
	case KEY_P: return Rml::Input::KI_P;
	case KEY_Q: return Rml::Input::KI_Q;
	case KEY_R: return Rml::Input::KI_R;
	case KEY_S: return Rml::Input::KI_S;
	case KEY_T: return Rml::Input::KI_T;
	case KEY_U: return Rml::Input::KI_U;
	case KEY_V: return Rml::Input::KI_V;
	case KEY_W: return Rml::Input::KI_W;
	case KEY_X: return Rml::Input::KI_X;
	case KEY_Y: return Rml::Input::KI_Y;
	case KEY_Z: return Rml::Input::KI_Z;
		// Special
	case KEY_SEMICOLON: return Rml::Input::KI_OEM_1;
	case KEY_PLUS: return Rml::Input::KI_OEM_PLUS;
	case KEY_COMMA: return Rml::Input::KI_OEM_COMMA;
	case KEY_MINUS: return Rml::Input::KI_OEM_MINUS;
	case KEY_PERIOD: return Rml::Input::KI_OEM_PERIOD;
	case KEY_SLASH: return Rml::Input::KI_OEM_2;
	case KEY_QUOTELEFT: return Rml::Input::KI_OEM_3;
	case KEY_BRACKETLEFT: return Rml::Input::KI_OEM_4;
	case KEY_BACKSLASH: return Rml::Input::KI_OEM_5;
	case KEY_BRACKETRIGHT: return Rml::Input::KI_OEM_6;
	case KEY_QUOTEDBL: return Rml::Input::KI_OEM_7;
		// Keypad
	case KEY_KP_0: return Rml::Input::KI_NUMPAD0;
	case KEY_KP_1: return Rml::Input::KI_NUMPAD1;
	case KEY_KP_2: return Rml::Input::KI_NUMPAD2;
	case KEY_KP_3: return Rml::Input::KI_NUMPAD3;
	case KEY_KP_4: return Rml::Input::KI_NUMPAD4;
	case KEY_KP_5: return Rml::Input::KI_NUMPAD5;
	case KEY_KP_6: return Rml::Input::KI_NUMPAD6;
	case KEY_KP_7: return Rml::Input::KI_NUMPAD7;
	case KEY_KP_8: return Rml::Input::KI_NUMPAD8;
	case KEY_KP_9: return Rml::Input::KI_NUMPAD9;
	case KEY_KP_ENTER: return Rml::Input::KI_NUMPADENTER;
	case KEY_KP_MULTIPLY: return Rml::Input::KI_MULTIPLY;
	case KEY_KP_ADD: return Rml::Input::KI_ADD;
	case KEY_KP_SUBTRACT: return Rml::Input::KI_SUBTRACT;
	case KEY_KP_PERIOD: return Rml::Input::KI_DECIMAL;
	case KEY_KP_DIVIDE: return Rml::Input::KI_DIVIDE;
		// Aux
	case KEY_BACKSPACE: return Rml::Input::KI_BACK;
	case KEY_TAB: return Rml::Input::KI_TAB;
	case KEY_CLEAR: return Rml::Input::KI_CLEAR;
	case KEY_ENTER: return Rml::Input::KI_RETURN;
	case KEY_PAUSE: return Rml::Input::KI_PAUSE;
	case KEY_CAPSLOCK: return Rml::Input::KI_CAPITAL;
	case KEY_PAGEUP: return Rml::Input::KI_PRIOR;
	case KEY_PAGEDOWN: return Rml::Input::KI_NEXT;
	case KEY_END: return Rml::Input::KI_END;
	case KEY_HOME: return Rml::Input::KI_HOME;
	case KEY_LEFT: return Rml::Input::KI_LEFT;
	case KEY_RIGHT: return Rml::Input::KI_RIGHT;
	case KEY_UP: return Rml::Input::KI_UP;
	case KEY_DOWN: return Rml::Input::KI_DOWN;
	case KEY_INSERT: return Rml::Input::KI_INSERT;
	case KEY_DELETE: return Rml::Input::KI_DELETE;
	case KEY_HELP: return Rml::Input::KI_HELP;
		// Function
	case KEY_F1: return Rml::Input::KI_F1;
	case KEY_F2: return Rml::Input::KI_F2;
	case KEY_F3: return Rml::Input::KI_F3;
	case KEY_F4: return Rml::Input::KI_F4;
	case KEY_F5: return Rml::Input::KI_F5;
	case KEY_F6: return Rml::Input::KI_F6;
	case KEY_F7: return Rml::Input::KI_F7;
	case KEY_F8: return Rml::Input::KI_F8;
	case KEY_F9: return Rml::Input::KI_F9;
	case KEY_F10: return Rml::Input::KI_F10;
	case KEY_F11: return Rml::Input::KI_F11;
	case KEY_F12: return Rml::Input::KI_F12;
	case KEY_F13: return Rml::Input::KI_F13;
	case KEY_F14: return Rml::Input::KI_F14;
	case KEY_F15: return Rml::Input::KI_F15;
	case KEY_NUMLOCK: return Rml::Input::KI_NUMLOCK;
	case KEY_SCROLLLOCK: return Rml::Input::KI_SCROLL;
		// There is no distinction in Godot.
	case KEY_SHIFT: return Rml::Input::KI_LSHIFT;
	case KEY_ALT: return Rml::Input::KI_LMENU;
	case KEY_CTRL: return Rml::Input::KI_LCONTROL;
	case KEY_META: return Rml::Input::KI_LMETA;
		// anything we didn't bind is unknown
	default: return Rml::Input::KI_UNKNOWN;
	}
}

void RmlNode::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("connect_event", "elementId", "eventName", "func"), &RmlNode::connect_event);
	ClassDB::bind_method(D_METHOD("get_document_path"), &RmlNode::get_document_path);
	ClassDB::bind_method(D_METHOD("set_document_path", "newPath"), &RmlNode::set_document_path);
	ClassDB::add_property("RmlNode", PropertyInfo(Variant::STRING, "m_DocumentPath", PROPERTY_HINT_FILE), "set_document_path", "get_document_path");
}

RmlNode::RmlNode()
{
	m_Context = nullptr;
	m_Document = nullptr;

	set_focus_mode(FOCUS_ALL);
	set_mouse_filter(MOUSE_FILTER_STOP);
	set_process_unhandled_key_input(true);
	UtilityFunctions::print("CREATED!");
}

RmlNode::~RmlNode()
{
}

void RmlNode::set_document_path(const String newPath)
{
	m_DocumentPath = newPath;
	UtilityFunctions::print("SET PATH!");
}

String RmlNode::get_document_path() const
{
	return m_DocumentPath;
}

void RmlNode::connect_event(const String elementId, const String eventName, Callable func)
{
	Rml::Element* elem = m_Document->GetElementById(Conversion::stringToStd(elementId));

	if (elem == nullptr)
	{
		UtilityFunctions::print("GDRML: No element named '", elementId, "' exists in the document.");
		return;
	}

	// hack: stupid but we got a mem violation so just make a new callable around what we were passed
	elem->AddEventListener(Conversion::stringToStd(eventName), new GDScriptEventListener(elem, new Callable(func)));
}

void RmlNode::_ready()
{
	UtilityFunctions::print("ITS READY!");
}

void RmlNode::_enter_tree()
{
	// don't run in the editor please
	//if (Engine::get_singleton()->is_editor_hint())
		//return;

		UtilityFunctions::print("ENTERED TREE!");
		
	if (RmlUIController::Instance() == nullptr)
	{
		UtilityFunctions::print("GDRML: !!!! RmlUIController didn't get initialized!");
		return;
	}

	m_Context = Rml::CreateContext("GodotNode_" + std::to_string(get_instance_id()), Rml::Vector2i(
		get_viewport_rect().get_size().width, get_viewport_rect().get_size().height
	));

	if (m_Context)
	{
		m_Document = m_Context->LoadDocument(Conversion::stringToStd(m_DocumentPath));
		m_Document->Show();
	}
	else
	{
		UtilityFunctions::print("GDRML: Failed to initialize context!");
	}
}

void RmlNode::_exit_tree()
{
	if (m_Document)
		m_Document->Close();
}

void RmlNode::_process(double deltaTime)
{
	if (m_Context) {
		//UtilityFunctions::print("updating rmlui");
		// I wish I had LateUpdate to mostly ensure that this ran after everything else
		m_Context->Update();

		// we don't really have any indication as to if we're idle or not with RML :shrug:
		queue_redraw();
	}
}

void RmlNode::_draw()
{
	if (m_Context) {
		UtilityFunctions::print("rendering rmlui");
		// set the current canvas item to our canvas item RID
		dynamic_cast<GDRenderInterface*>(Rml::GetRenderInterface())->SetCanvasItem(get_canvas_item());

		// TODO: update the context size when things get resized

		m_Context->Render();
	}
}

void RmlNode::_gui_input(const Ref<InputEvent>& event)
{
	if (!m_Context)
		return;

	// small maybe optimization? don't check against motion and button if we aren't even a mouse event.
	const InputEventMouse* mouseEvent = Object::cast_to<const InputEventMouse>(*event);

	if (mouseEvent) {
		const InputEventMouseMotion* mouseMotionEvent = Object::cast_to<const InputEventMouseMotion>(mouseEvent);

		if (mouseMotionEvent)
		{
			m_Context->ProcessMouseMove(mouseMotionEvent->get_position().x, mouseMotionEvent->get_position().y, 0);
			return;
		}
		else
		{
			const InputEventMouseButton* mouseButtonEvent = Object::cast_to<const InputEventMouseButton>(mouseEvent);

			if (mouseButtonEvent->is_pressed())
			{
				switch (mouseButtonEvent->get_button_index())
				{
				case MouseButton::MOUSE_BUTTON_LEFT:
					m_Context->ProcessMouseButtonDown(0, 0);
					break;
				case MouseButton::MOUSE_BUTTON_RIGHT:
					m_Context->ProcessMouseButtonDown(1, 0);
					break;
				case MouseButton::MOUSE_BUTTON_MIDDLE:
					m_Context->ProcessMouseButtonDown(2, 0);
					break;
				}
			}
			else
			{
				switch (mouseButtonEvent->get_button_index())
				{
				case MouseButton::MOUSE_BUTTON_LEFT:
					m_Context->ProcessMouseButtonUp(0, 0);
					break;
				case MouseButton::MOUSE_BUTTON_RIGHT:
					m_Context->ProcessMouseButtonUp(1, 0);
					break;
				case MouseButton::MOUSE_BUTTON_MIDDLE:
					m_Context->ProcessMouseButtonUp(2, 0);
					break;
				}
			}
		}

		if (m_Context->IsMouseInteracting())
		{
			accept_event();
			return;
		}
	}

	bool eventConsumed = false;
	const InputEventKey* keyEvent = Object::cast_to<const InputEventKey>(*event);
	if (keyEvent)
	{
		// todo: modifiers
		int modifierState = 0;

		if (keyEvent->is_shift_pressed())
			modifierState |= Rml::Input::KeyModifier::KM_SHIFT;
		if (keyEvent->is_alt_pressed())
			modifierState |= Rml::Input::KeyModifier::KM_ALT;
		if (keyEvent->is_ctrl_pressed())
			modifierState |= Rml::Input::KeyModifier::KM_CTRL;

		if (keyEvent->is_pressed())
		{
			eventConsumed = m_Context->ProcessKeyDown(GDKeyToRml(keyEvent->get_keycode()), modifierState);
			
			// Allow unicode handling if:
			// * No Modifiers are pressed (except shift)
			bool allow_unicode_handling = !(keyEvent->is_ctrl_pressed() || keyEvent->is_alt_pressed() || keyEvent->is_meta_pressed());

			if (allow_unicode_handling && !eventConsumed) {
				// Handle Unicode if no modifiers are active.
				char32_t ucodestr[2] = { (char32_t)keyEvent->get_unicode(), 0 };
				eventConsumed = m_Context->ProcessTextInput(Conversion::stringToStd(ucodestr));
			}
		}
		else
		{
			eventConsumed = m_Context->ProcessKeyUp(GDKeyToRml(keyEvent->get_keycode()), modifierState);
		}

		if (eventConsumed)
		{
			accept_event();
			return;
		}
	}
}
