/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: sandbox.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/core.h>


/////////////////////////////
// Vital: Manager: Sandbox //
/////////////////////////////

// TODO: Scope under Manager namespace
namespace Vital::Engine {
	class Canvas;
}

namespace Vital::Manager {
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


			// Managers //
			void ready();
			void process(double delta);
			#if defined(Vital_SDK_Client)
			void draw(Engine::Canvas* canvas);
			void input(godot::Ref<godot::InputEvent> event);
			#endif

			template<typename... Args>
			void signal(const std::string& name, Args&&... args) {
				static_assert((std::is_same_v<std::decay_t<Args>, Tool::StackValue> && ...), "Sandbox::signal() expects StackValue arguments");
				Tool::Stack stack;
				stack.array.reserve(sizeof...(Args));
				(stack.array.emplace_back(std::forward<Args>(args)), ...);
				Tool::Event::emit(name, stack);
				if (!vm || !vm -> is_reference(signal_reference)) return;
				vm -> get_reference(signal_reference, true);
				vm -> push_value(name);
				for (auto& value : stack.array) {
					vm -> push_value(value);
				}
				vm -> pcall(sizeof...(Args) + 1, 0);
			}

	
			// Getters //
			Vital::Sandbox::Machine* get_vm();
	};	
}