/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: public: lua.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/core.h>
#include <Vital.extension/Engine/public/canvas.h>


/////////////////////////////////
// Vital: Godot: Sandbox: Lua //
/////////////////////////////////

namespace Vital::Godot::Sandbox::Lua {
	class Singleton {
		protected:
			Singleton();
			~Singleton();
		public:
			static Singleton* fetch();
			void ready();
			void process(double delta);
			void draw(Vital::Godot::Canvas* canvas);
	};	
}