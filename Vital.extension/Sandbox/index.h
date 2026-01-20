/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: index.h
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


////////////////////////////
// Vital: Godot: Sandbox //
////////////////////////////

namespace Vital::Godot::Sandbox {
	class Singleton;
	class Singleton {
		protected:
			inline static Singleton* instance = nullptr;
		public:
			Singleton();
			~Singleton();
			static Singleton* fetch();
			void ready();
			void process(double delta);
			#if defined(Vital_SDK_Client)
			void draw(Vital::Godot::Canvas* canvas);
			#endif
	};	
}