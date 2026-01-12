#pragma once

#include <RmlUi/Core/SystemInterface.h>

namespace gdrml {

class GDSystemInterface : public Rml::SystemInterface
{
public:
	GDSystemInterface();

	double GetElapsedTime() override;
	bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;
	int TranslateString(Rml::String& translated, const Rml::String& input) override;
};

}
