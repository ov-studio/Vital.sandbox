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


/////////////////////////////
// Vital: Engine: Sandbox //
/////////////////////////////

namespace Vital::Engine {
	class Canvas;
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


			// Managers //
			void ready();
			void process(double delta);
			#if defined(Vital_SDK_Client)
			void draw(Canvas* canvas);
			void input(godot::Ref<godot::InputEvent> event);
			#endif

			template<typename... Args>
			void signal(const std::string& name, Args&&... args) {
				if (!vm) return;
				const std::string ref = "vital.network:execute";
				if (vm -> is_reference(ref)) {
					vm -> get_reference(ref, true);
					vm -> push_value(name);
					(vm -> push_value(std::forward<Args>(args)), ...);
					vm -> pcall(sizeof...(Args) + 1, 0);
				}
			}
	};	
}