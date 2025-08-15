/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: private: sandbox.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Engine/public/sandbox.h>
#include <Sandbox/public/lua.h>


//////////////
// Sandbox //
//////////////

Sandbox::Sandbox() {
	godot::UtilityFunctions::print("Initialized example class");
    ExampleLUA::fetch();
}

void Sandbox::_bind_methods() {

}

void Sandbox::_process(double delta) {

}
