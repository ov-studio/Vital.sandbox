/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Engine: public: lua.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Lua Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Engine/public/sandbox.h>


/////////////////
// ExampleLUA //
/////////////////

class ExampleLUA {
	protected:
		ExampleLUA();
		~ExampleLUA();
	public:
		static ExampleLUA* singleton_instance;
		static ExampleLUA* fetch();

		void process(double delta);
};
