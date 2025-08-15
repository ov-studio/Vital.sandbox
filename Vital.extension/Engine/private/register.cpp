/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: register.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Register Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include "register.h"
#include "sandbox.h"
#include <godot_cpp/godot.hpp>


//////////////////
// Registerers //
//////////////////

void initialize_gdextension_types(godot::ModuleInitializationLevel p_level) {
	if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) return;
	godot::ClassDB::register_class<Sandbox>();
}

void uninitialize_gdextension_types(godot::ModuleInitializationLevel p_level) {
	if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) return;
}

extern "C" {
	GDExtensionBool GDE_EXPORT example_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
		godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
		init_obj.register_initializer(initialize_gdextension_types);
		init_obj.register_terminator(uninitialize_gdextension_types);
		init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);
		return init_obj.init();
	}
}
