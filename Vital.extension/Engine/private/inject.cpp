/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: inject.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Inject Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Engine/public/inject.h>
#include <Vital.sandbox/Tool/event.h>


//////////////
// Injects //
//////////////

void initialize_gdextension_types(godot::ModuleInitializationLevel p_level) {
	if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) return;
	godot::ClassDB::register_class<Vital::Godot::Core>();
	#if defined(Vital_SDK_Client)
	godot::ClassDB::register_class<Vital::Godot::Canvas>(true);
	godot::ClassDB::register_class<Vital::Godot::RenderTarget>(true);
	#endif

	Vital::System::Event::bind("Godot:Core:@ready", [](Vital::Tool::Stack arguments) -> void {
		#if defined(Vital_SDK_Client)
		Vital::Godot::Core::get_environment();
		Vital::Godot::Canvas::get_singleton();

		// TODO: REMOVE LATER
		auto* webview = memnew(Vital::Godot::Webview);
		webview -> load_from_url("https://ko-fi.com/ovstudio");
		#endif
		Vital::Godot::Sandbox::Lua::Singleton::fetch() -> ready();
	});
}

void uninitialize_gdextension_types(godot::ModuleInitializationLevel p_level) {
	if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) return;
}

extern "C" {
	GDExtensionBool GDE_EXPORT vital_sandbox_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
		godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
		init_obj.register_initializer(initialize_gdextension_types);
		init_obj.register_terminator(uninitialize_gdextension_types);
		init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);
		return init_obj.init();
	}
}
