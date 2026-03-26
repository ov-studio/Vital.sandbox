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
			inline static const std::string signal_reference = "vital.network:execute";
		private:
			Vital::Sandbox::Machine* vm = nullptr;
		public:
			// Instantiators //
			Sandbox();
			~Sandbox();


			// Utils //
			static Sandbox* get_singleton();
            static void free_singleton();


			// Getters //
			Vital::Sandbox::Machine* get_vm();


			// APIs //
			void ready();
			void process(double delta);
			#if defined(Vital_SDK_Client)
			void draw(Canvas* canvas);
			void input(godot::Ref<godot::InputEvent> event);
			#endif

			template<typename... Args>
			void signal(const std::string& name, Args&&... args) {
				static_assert((std::is_same_v<std::decay_t<Args>, Vital::Tool::StackValue> && ...), "Sandbox::signal() expects StackValue arguments");
				Vital::Tool::Stack stack;
				stack.array.reserve(sizeof...(Args));
				(stack.array.emplace_back(std::forward<Args>(args)), ...);
				Vital::Tool::Event::emit(name, stack);
				if (!vm || !vm -> is_reference(signal_reference)) return;
				vm -> get_reference(signal_reference, true);
				vm -> push_value(name);
				for (auto& value : stack.array) {
					vm -> push_value(value);
				}
				vm -> pcall(sizeof...(Args) + 1, 0);
			}
	};	
}