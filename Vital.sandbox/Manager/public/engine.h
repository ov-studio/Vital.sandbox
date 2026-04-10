/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: engine.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Engine Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>
#include <Vital.sandbox/Engine/public/network.h>
#include <Vital.sandbox/Manager/public/asset.h>
#include <Vital.sandbox/Engine/public/canvas.h>
#include <Vital.sandbox/Engine/public/model.h>
#include <Vital.sandbox/Engine/public/webview.h>
#include <Vital.sandbox/Engine/public/font.h>
#include <Vital.sandbox/Engine/public/texture.h>
#include <Vital.sandbox/Engine/public/rendertarget.h>
#include <Vital.sandbox/Engine/public/console.h>
#include <Vital.sandbox/Manager/public/sandbox.h>
#include <Vital.sandbox/Manager/public/resource.h>


/////////////////////////////
// Vital: Manager: Engine //
/////////////////////////////

void initialize_vital_events();

inline void initialize_gdextension_types(godot::ModuleInitializationLevel p_level) {
    if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) return;
    godot::ClassDB::register_class<Vital::Engine::Core>();
    godot::ClassDB::register_class<Vital::Engine::NetworkNode>(true);
    godot::ClassDB::register_class<Vital::Engine::ModelSpawnerDelegate>(true);
    godot::ClassDB::register_class<Vital::Engine::Model>(true);
    #if defined(Vital_SDK_Client)
    godot::ClassDB::register_class<Vital::Engine::Canvas>(true);
    godot::ClassDB::register_class<Vital::Engine::Rendertarget>(true);
    godot::ClassDB::register_class<Vital::Engine::Webview>(true);
    #endif
    initialize_vital_events();
}

inline void uninitialize_gdextension_types(godot::ModuleInitializationLevel p_level) {
    if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) return;
}

extern "C" {
    inline GDExtensionBool GDE_EXPORT vital_sandbox_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization) {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
        init_obj.register_initializer(initialize_gdextension_types);
        init_obj.register_terminator(uninitialize_gdextension_types);
        init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);
        return init_obj.init();
    }
}