/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Vital: entrypoint.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: SDK Entrypoint
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Vital/engine.h>
#include <Vital.sandbox/Vital/manager.h>


////////////////
// SDK Hooks //
////////////////

void vsdk_initialize();

inline void initialize_gdextension_types(godot::ModuleInitializationLevel p_level) {
    if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) return;
    godot::ClassDB::register_class<Vital::Engine::Core>();
    godot::ClassDB::register_class<Vital::Engine::Network>(true);
    godot::ClassDB::register_class<Vital::Engine::ModelSpawnerDelegate>(true);
    godot::ClassDB::register_class<Vital::Engine::Model>(true);
    #if defined(VSDK_Client)
    godot::ClassDB::register_class<Vital::Engine::Canvas>(true);
    godot::ClassDB::register_class<Vital::Engine::Camera>(true);
    godot::ClassDB::register_class<Vital::Engine::Webview>(true);
    godot::ClassDB::register_class<Vital::Engine::Rendertarget>(true);
    #endif
    vsdk_initialize();
}

inline void uninitialize_gdextension_types(godot::ModuleInitializationLevel p_level) {
    if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) return;
}

extern "C" {
    inline GDExtensionBool GDE_EXPORT vsdk_entrypoint(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization) {
        Vital::Tool::main_thread_id = std::this_thread::get_id();
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
        init_obj.register_initializer(initialize_gdextension_types);
        init_obj.register_terminator(uninitialize_gdextension_types);
        init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);
        return init_obj.init();
    }
}