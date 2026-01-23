/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: index.h
     Author: ov-studio
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

namespace Vital::Godot {
	class Sandbox;
	class Sandbox : public godot::Node {
		protected:
			inline static Sandbox* singleton = nullptr;
		private:
			Vital::Sandbox::Machine* vm = nullptr;
		public:
			// Instantiators //
			Sandbox();
			~Sandbox();


			// Utils //
			static Sandbox* get_singleton();
            static void free_singleton();


			// APIs //
			void ready();
			void process(double delta);
			#if defined(Vital_SDK_Client)
			void input(godot::Ref<godot::InputEvent> event);
			void draw(Canvas* canvas);
			#endif
	};	
}