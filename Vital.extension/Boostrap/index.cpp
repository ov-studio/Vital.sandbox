/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Boostrap: index.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Boostrap Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Boostrap/index.h>


///////////
// APIs //
///////////

void initialize_gdextension_types(godot::ModuleInitializationLevel p_level) {
	if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) return;
	godot::ClassDB::register_class<Vital::Godot::Core>();
	#if defined(Vital_SDK_Client)
	godot::ClassDB::register_class<Vital::Godot::Canvas>(true);
	godot::ClassDB::register_class<Vital::Godot::RenderTarget>(true);
	godot::ClassDB::register_class<Vital::Godot::Webview>(true);
	#endif
	initialize_vital_events();
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
