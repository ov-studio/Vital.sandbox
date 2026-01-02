/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: canvas.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Canvas Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Engine/public/singleton.h>


///////////////////////////
// Vital: Godot: Engine //
///////////////////////////

namespace Vital::Godot::Engine {
	class Canvas : public godot::Node2D  {
		GDCLASS(Canvas, godot::Node2D)
		protected:
			static void _bind_methods() {};
		public:
			// Instantiators //
			Canvas();
			~Canvas() override = default;
			void _ready() override;
			void _process(double delta) override;
			void _draw() override;
	};
}