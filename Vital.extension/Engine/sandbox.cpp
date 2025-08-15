#pragma once
#include "sandbox.h"
#include "../Sandbox/lua.h"


Sandbox::Sandbox() {
	godot::UtilityFunctions::print("Initialized example class");
    ExampleLUA::fetch();
}

void Sandbox::_bind_methods() {

}

void Sandbox::_process(double delta) {

}
