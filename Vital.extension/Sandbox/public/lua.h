/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: public: lua.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Lua Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Engine/public/index.h>


/////////////////////////////////
// Vital: Godot: Sandbox: Lua //
/////////////////////////////////

namespace Vital::Godot::Sandbox {
	class Lua {
		protected:
			Lua();
			~Lua();
		public:
			static Lua* fetch();
			void process(double delta);
	};	
}