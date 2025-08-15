#pragma once

class ExampleLUA {
	protected:
		ExampleLUA();
		~ExampleLUA();
	public:
		static ExampleLUA* singleton_instance;
		static ExampleLUA* fetch();

		void process(double delta);
};
